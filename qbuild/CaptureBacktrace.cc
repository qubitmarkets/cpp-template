// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#include "qbuild/CaptureBacktrace.h"
#include "qbuild/ansi_colors.h"
#include "qbuild/compiler.h"
#include <backtrace.h>  // libbacktrace
#include <cxxabi.h>     // abi
#include <libgen.h>     // dirname, basename
#include <limits.h>     // INT_MAX
#include <stdlib.h>     // malloc, free
#include <string.h>     // strdup
#include <unistd.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// ------------------------
// Forward declarations
//
backtrace_state* __bt_state = nullptr;
bool backtrace_init(const char* filename);

// ------------------------
// PrintBacktrace
//
struct PrintBacktrace {
    int skip_frames = 1;
    int frame_idx = 0;
    bool has_first_line = false;
    static bool can_skip(const char* function) {
        const char* to_skip[]{
            "__GI___dl_iterate_phdr",  // Internal function used by libbacktrace
            "__GI___mmap64",
            "__GI___clock_gettime",
            "__pthread_kill_implementation",
            "__GI_raise",
            "__GI_abort",
            "__internal_syscall_cancel",
            "__syscall_cancel",
            "___interceptor_clock_gettime",
            "CaptureBacktrace::print() const",
            "sig_handler",
            nullptr,  // Sentinel
        };
        for (const char** skip = to_skip; *skip != nullptr; ++skip) {
            if (strcmp(function, *skip) == 0) {
                return true;
            }
        }
        return false;
    }

    static int on_bt_frame(void* self_, uintptr_t x, const char* filepath, int lineno, const char* c_funcname) {
        PrintBacktrace* self = (PrintBacktrace*)self_;
        if (self->frame_idx < self->skip_frames) {
            self->frame_idx += 1;
            return 0;
        }
        if (c_funcname && can_skip(c_funcname)) {
            if (self->frame_idx > 1) {
                self->frame_idx += 1;
            }
            return 0;
        }

        /// demangle function name
        const char* func_name = c_funcname;
        int status;
        char* demangled = abi::__cxa_demangle(c_funcname, nullptr, nullptr, &status);
        if (status == 0) {
            func_name = demangled;
        }

        if (!filepath || !func_name || can_skip(func_name)) {
            // When compiling with debug compiler, we have an additional undefined stack frame at the beginning.  Skip it, so we have
            // consistent results in debug and release.
            if (self->frame_idx > 1) {
                self->frame_idx += 1;
            }
            if (demangled) {
                ::free((void*)demangled);
            }
            return 0;
        }
        char* dir_ = strdup(filepath);
        auto dir = ::dirname(dir_);
        char* filename_ = strdup(filepath);
        auto filename = ::basename(filename_);

        // Print the first line
        bool first_line = false;
        if (!self->has_first_line) {
            first_line = true;
            self->has_first_line = true;
        }
        if (first_line) {
            eprintf("Location: " COLOR_PURPLE "%s" COLOR_NONE ":" COLOR_PURPLE "%d" COLOR_NONE "\n", filename, lineno);
            eprintf("\n  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ BACKTRACE ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
            eprintf("                               " COLOR_CYAN " ⋮ %d frames hidden ⋮" COLOR_NONE "\n", self->skip_frames);
        }

        // Skip the boring tail
        if (func_name && strcmp(func_name, "__libc_start_call_main") == 0) {
            self->skip_frames = INT_MAX;
        } else {
            // Print the frame
            auto func_color = first_line ? COLOR_PINK : COLOR_GREEN;
            eprintf("% 4d: %s%s" COLOR_NONE
                    "\n"
                    "      at " COLOR_BLUE "%s/" COLOR_RED "%s" COLOR_NONE ":%d" COLOR_NONE "\n",
                    self->frame_idx, func_color, func_name, dir, filename, lineno);
            self->frame_idx += 1;
        }
        ::free(filename_);
        ::free(dir_);
        if (demangled) {
            ::free((void*)demangled);
        }
        return 0;
    }

    static void on_bt_error(void*, const char* msg, int errnum) {
        eprintf("Error %d occurred when getting the stacktrace: %s", errnum, msg);
    }
};

// ------------------------
// PrintBacktrace
//

void CaptureBacktrace::init(const char* exec_filename) {
    backtrace_init(exec_filename);
}

CaptureBacktrace::CaptureBacktrace(int stack_max_) : stack_max(stack_max_) {
    stack = new uintptr_t[stack_max];
}

CaptureBacktrace::~CaptureBacktrace() {
    delete[] stack;
}

int CaptureBacktrace::on_bt_frame(void* self_, uintptr_t pc) {
    CaptureBacktrace* self = (CaptureBacktrace*)self_;
    self->stack[self->stack_len++] = pc;
    return 0;
}

void CaptureBacktrace::on_bt_error(void* self, const char* msg, int errnum) {
    eprintf("backtrace error %d: %s\n", errnum, msg);
}

NOINLINE
void CaptureBacktrace::print() const {
    if (stack_len == 0) {
        capture(1);
    }
    PrintBacktrace p;
    for (int i = 0; i < stack_len; ++i) {
        int rv = backtrace_pcinfo(__bt_state, stack[i], &PrintBacktrace::on_bt_frame, &PrintBacktrace::on_bt_error, &p);
    }
    eprintf("\n");
    if (p.frame_idx <= 1) {
        eprintf("CaptureBacktrace: no stack trace available\n");
    }
}

const char* get_executable() {
    // Get cmdline the hacky way, argv[0] is after environ
    int i;
    for (i = 0; ::environ[i] != nullptr; ++i) {
    }
    const char* cmd = ::environ[i - 1];
    cmd += strlen(cmd) + 1;
    return cmd;
}

NOINLINE
void CaptureBacktrace::capture(int skip_frames) const {
    if unlikely (__bt_state == nullptr) {
        auto cmd = get_executable();
        CaptureBacktrace::init(cmd);
    }
    backtrace_simple((backtrace_state*)__bt_state, skip_frames, &CaptureBacktrace::on_bt_frame, &CaptureBacktrace::on_bt_error,
                     (void*)this);
}

// -----------------------------
// Adapted from https://tjysdsg.github.io/libbacktrace/

void bt_error_callback_create(void* data, const char* msg, int errnum) {
    printf("Error %d occurred when initializing the stacktrace: %s", errnum, msg);
    bool* status = (bool*)data;
    *status = false;
}

bool backtrace_init(const char* filename) {
    bool status = true;
    __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, (void*)status);
    return status;
}

NOINLINE
void backtrace_print(int skip_frames) {
    if (!__bt_state) {  /// make sure init_back_trace() is called
        printf(
            "Make sure init_back_trace() is called before calling "
            "print_stack_trace()\n");
        abort();
    }
    backtrace_full((backtrace_state*)__bt_state, skip_frames + 1, &PrintBacktrace::on_bt_frame, &PrintBacktrace::on_bt_error, nullptr);
    eprintf("\n");
}

// ---------------------------
