# genie's toolchains and libraries for this repo: where they are installed, and which compilers it uses.
# Sourced by build-run, tests.sh, tidy.sh and prereqs.sh. etc/ is shared: repos made from cpp-template link to it, so
# this is the one place to change for every repo. Each value can be overridden from the environment.
#
# The CMake presets (debug.gcc16, release.sanitize.clang23, ...) come from genie's genie-presets package, included by
# CMakePresets.json from $INFRA_ROOT/share/genie/; a new compiler appears there when genie builds it.

INFRA_ROOT=${INFRA_ROOT-/opt/infra.1}
# build-run's compiler when $compiler is not set
DEFAULT_COMPILER=${DEFAULT_COMPILER-clang23}
# tests.sh builds and tests each, debug and release, with and without sanitize
TEST_COMPILERS=${TEST_COMPILERS-"gcc16 clang23"}
