"""
Pretty-printers for abseil containers.
Tailored from [mongoDB's GDB printer](https://github.com/mongodb/mongo/blob/master/buildscripts/gdb/mongo_printers.py)

To use:
1. Change `ABSL_OPTION_INLINE_NAMESPACE_NAME` to your abseil version.
2. Add `source /path/to/absl_pretty_printers.py` to your ~/.gdbinit.
"""


# Change this to your abseil version!
# See more: https://github.com/abseil/abseil-cpp/blob/929c17cf481222c35ff1652498994871120e832a/absl/base/options.h#L203
ABSL_OPTION_INLINE_NAMESPACE_NAME = "lts_20250512"

import os
import sys
from pathlib import Path

import gdb
import gdb.printing


if sys.version_info[0] < 3:
    raise gdb.GdbError(
        "Extension only support Python 3. Your GDB was compiled against Python 2"
    )

def lookup_type(gdb_type_str: str) -> gdb.Type:
    """
    Try to find the type object from string.

    GDB says it searches the global blocks, however this appear not to be the
    case or at least it doesn't search all global blocks, sometimes it required
    to get the global block based off the current frame.
    """
    global MAIN_GLOBAL_BLOCK

    exceptions = []
    try:
        return gdb.lookup_type(gdb_type_str)
    except Exception as exc:
        exceptions.append(exc)

    if MAIN_GLOBAL_BLOCK is None:
        MAIN_GLOBAL_BLOCK = gdb.lookup_symbol("main")[0].symtab.global_block()

    try:
        return gdb.lookup_type(gdb_type_str, MAIN_GLOBAL_BLOCK)
    except Exception as exc:
        exceptions.append(exc)

    raise gdb.error("Failed to get type, tried:\n%s" % "\n".join([str(exc) for exc in exceptions]))

###################################################################################################
#
# Pretty-Printers
#
###################################################################################################


class BoostOptionalPrinter(object):
    """Pretty-printer for boost::optional."""

    def __init__(self, val):
        """Initialize BoostOptionalPriner."""
        self.val = val

    def to_string(self):
        """Return data for printing."""
        return get_boost_optional(self.val)


MAX_DB_NAME_LENGTH = 63
TENANT_ID_MASK = 0x80
OBJECT_ID_WIDTH = 12


def extract_tenant_id(data):
    raw_bytes = [int(data[i]) for i in range(1, OBJECT_ID_WIDTH + 1)]
    return "".join([hex(b & 0xFF)[2:].zfill(2) for b in raw_bytes])


def is_small_string(flags):
    return bool(flags & 0b00000010)


def small_string_size(flags):
    return flags >> 2


def absl_insert_version_after_absl(cpp_name):
    """Insert version inline namespace after the first `absl` namespace found in the given string."""

    absl_ns_str = "absl::"
    absl_ns_start = cpp_name.find(absl_ns_str)
    if absl_ns_start == -1:
        raise ValueError("No `absl` namespace found in " + cpp_name)

    absl_ns_end = absl_ns_start + len(absl_ns_str)

    return (
        cpp_name[:absl_ns_end] + ABSL_OPTION_INLINE_NAMESPACE_NAME + "::" + cpp_name[absl_ns_end:]
    )


def absl_get_settings(val):
    """Gets the settings_ field for abseil (flat/node)_hash_(map/set)."""
    try:
        common_fields_storage_type = gdb.lookup_type(
            absl_insert_version_after_absl(
                "absl::container_internal::internal_compressed_tuple::Storage"
            )
            + absl_insert_version_after_absl("<absl::container_internal::CommonFields, 0ul, false>")
        )
    except gdb.error as err:
        if not err.args[0].startswith("No type named "):
            raise err

        # Abseil uses `inline namespace lts_20230802 { ... }` for its container types. This
        # can inhibit GDB from resolving type names when the inline namespace appears within
        # a template argument.
        common_fields_storage_type = gdb.lookup_type(
            absl_insert_version_after_absl(
                "absl::container_internal::internal_compressed_tuple::Storage<"
                + absl_insert_version_after_absl("absl::container_internal::CommonFields, 0ul, false>")
            )
        )

    # The Hash, Eq, or Alloc functors may not be zero-sized objects.
    # mongo::LogicalSessionIdHash is one such example. An explicit cast is needed to
    # disambiguate which `value` member variable of the CompressedTuple is to be accessed.
    return val["settings_"].cast(common_fields_storage_type)["value"]


def absl_container_size(settings):
    try:
        return settings["compressed_tuple_"]["value"]
    except:
        return settings["capacity_"]


def absl_get_nodes(val):
    """Return a generator of every node in absl::container_internal::raw_hash_set and derived classes."""
    settings = absl_get_settings(val)

    size = absl_container_size(settings)
    if size == 0:
        return

    capacity = int(settings["capacity_"])
    heap = settings["heap_or_soo_"]["heap"]
    ctrl = heap["control"]

    # Derive the underlying type stored in the container.
    slot_type = lookup_type(str(val.type.strip_typedefs()) + "::slot_type").strip_typedefs()

    # Using the array of ctrl bytes, search for in-use slots and return them
    # https://github.com/abseil/abseil-cpp/blob/8a3caf7dea955b513a6c1b572a2423c6b4213402/absl/container/internal/raw_hash_set.h#L2108-L2113
    for item in range(capacity):
        ctrl_t = int(ctrl[item])
        if ctrl_t >= 0:
            yield heap["slot_array"].cast(slot_type.pointer())[item]


class AbslHashSetPrinterBase(object):
    """Pretty-printer base class for absl::[node/flat]_hash_set<>."""

    def __init__(self, val, to_str):
        """Initialize absl::[node/flat]_hash_set."""
        self.val = val
        self.to_str = to_str

    @staticmethod
    def display_hint():
        """Display hint."""
        return "array"

    def to_string(self):
        """Return absl::[node/flat]_hash_set for printing."""
        return "absl::%s_hash_set<%s> with %s elems " % (
            self.to_str,
            self.val.type.template_argument(0),
            absl_container_size(absl_get_settings(self.val)),
        )


class AbslNodeHashSetPrinter(AbslHashSetPrinterBase):
    """Pretty-printer for absl::node_hash_set<>."""

    def __init__(self, val):
        """Initialize absl::node_hash_set."""
        AbslHashSetPrinterBase.__init__(self, val, "node")

    def children(self):
        """Children."""
        count = 0
        for val in absl_get_nodes(self.val):
            yield (str(count), val.dereference())
            count += 1


class AbslFlatHashSetPrinter(AbslHashSetPrinterBase):
    """Pretty-printer for absl::flat_hash_set<>."""

    def __init__(self, val):
        """Initialize absl::flat_hash_set."""
        AbslHashSetPrinterBase.__init__(self, val, "flat")

    def children(self):
        """Children."""
        count = 0
        for val in absl_get_nodes(self.val):
            yield (str(count), val.reference_value())
            count += 1


class AbslHashMapPrinterBase(object):
    """Pretty-printer base class for absl::[node/flat]_hash_map<>."""

    def __init__(self, val, to_str):
        """Initialize absl::[node/flat]_hash_map."""
        self.val = val
        self.to_str = to_str

    @staticmethod
    def display_hint():
        """Display hint."""
        return "map"

    def to_string(self):
        """Return absl::[node/flat]_hash_map for printing."""
        return "absl::%s_hash_map<%s, %s> with %s elems " % (
            self.to_str,
            self.val.type.template_argument(0),
            self.val.type.template_argument(1),
            absl_container_size(absl_get_settings(self.val)),
        )


class AbslNodeHashMapPrinter(AbslHashMapPrinterBase):
    """Pretty-printer for absl::node_hash_map<>."""

    def __init__(self, val):
        """Initialize absl::node_hash_map."""
        AbslHashMapPrinterBase.__init__(self, val, "node")

    def children(self):
        """Children."""
        for kvp in absl_get_nodes(self.val):
            yield ("key", kvp["first"])
            yield ("value", kvp["second"])


class AbslFlatHashMapPrinter(AbslHashMapPrinterBase):
    """Pretty-printer for absl::flat_hash_map<>."""

    def __init__(self, val):
        """Initialize absl::flat_hash_map."""
        AbslHashMapPrinterBase.__init__(self, val, "flat")

    def children(self):
        """Children."""
        for kvp in absl_get_nodes(self.val):
            yield ("key", kvp["key"])
            yield ("value", kvp["value"]["second"])


def find_match_brackets(search, opening="<", closing=">"):
    """Return the index of the closing bracket that matches the first opening bracket.

    Return -1 if no last matching bracket is found, i.e. not a template.

    Example:
        'Foo<T>::iterator<U>''
        returns 5

    """
    index = search.find(opening)
    if index == -1:
        return -1

    start = index + 1
    count = 1
    str_len = len(search)
    for index in range(start, str_len):
        char = search[index]

        if char == opening:
            count += 1
        elif char == closing:
            count -= 1

        if count == 0:
            return index

    return -1


class MongoSubPrettyPrinter(gdb.printing.SubPrettyPrinter):
    """Sub pretty printer managed by the pretty-printer collection."""

    def __init__(self, name, prefix, is_template, printer):
        """Initialize MongoSubPrettyPrinter."""
        super(MongoSubPrettyPrinter, self).__init__(name)
        self.prefix = prefix
        self.printer = printer
        self.is_template = is_template


class MongoPrettyPrinterCollection(gdb.printing.PrettyPrinter):
    """MongoDB-specific printer printer collection that ignores subtypes.

    It will match 'HashTable<T> but not 'HashTable<T>::iterator' when asked for 'HashTable'.
    """

    def __init__(self):
        """Initialize MongoPrettyPrinterCollection."""
        super(MongoPrettyPrinterCollection, self).__init__("mongo", [])

    def add(self, name, prefix, is_template, printer):
        """Add a subprinter."""
        self.subprinters.append(MongoSubPrettyPrinter(name, prefix, is_template, printer))

    def __call__(self, val):
        """Return matched printer type."""

        # Get the type name.
        lookup_tag = gdb.types.get_basic_type(val.type).tag
        if not lookup_tag:
            lookup_tag = val.type.name
        if not lookup_tag:
            return None

        index = find_match_brackets(lookup_tag)

        for printer in self.subprinters:
            if not printer.enabled:
                continue
            # Ignore subtypes of templated classes.
            # We do not want HashTable<T>::iterator as an example, just HashTable<T>
            if printer.is_template:
                if index + 1 == len(lookup_tag) and lookup_tag.find(printer.prefix) == 0:
                    return printer.printer(val)
            elif lookup_tag == printer.prefix:
                return printer.printer(val)

        return None

def build_pretty_printer():
    """Build a pretty printer."""
    pp = MongoPrettyPrinterCollection()
    pp.add(
        "node_hash_map",
        absl_insert_version_after_absl("absl::node_hash_map"),
        True,
        AbslNodeHashMapPrinter,
    )
    pp.add(
        "node_hash_set",
        absl_insert_version_after_absl("absl::node_hash_set"),
        True,
        AbslNodeHashSetPrinter,
    )
    pp.add(
        "flat_hash_map",
        absl_insert_version_after_absl("absl::flat_hash_map"),
        True,
        AbslFlatHashMapPrinter,
    )
    pp.add(
        "flat_hash_set",
        absl_insert_version_after_absl("absl::flat_hash_set"),
        True,
        AbslFlatHashSetPrinter,
    )
    pp.add("boost::optional", "boost::optional", True, BoostOptionalPrinter)

    return pp


###################################################################################################
#
# Setup
#
###################################################################################################

# Register pretty-printers, replace existing mongo printers
gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer(), True)

print("Abseil GDB pretty-printers loaded")

