// Copyright (c) 2025 Qubit Markets Pte. Ltd.

#include "qbuild/CaptureBacktrace.h"
#include "qbuild/Callback.h"
#include "qbuild/ansi_colors.h"
#include "qbuild/compiler.h"
#include <backtrace.h>  // libbacktrace
#include <cxxabi.h>     // abi
#include <libgen.h>     // dirname, basename
#include <limits.h>     // INT_MAX
#include <stdlib.h>     // malloc, free
#include <string.h>     // strdup
#include <unistd.h>
#include <cstdarg>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// ------------------------
// Forward declarations
//
backtrace_state* __bt_state = nullptr;
bool backtrace_init(const char* filename);
PrintToStderrCB CaptureBacktrace::print_to_stderr_cb{};

// ------------------------
// PrintBacktrace
//
struct PrintBacktrace {
    PrintBacktrace(PrintToStderrCB print_to_stderr_cb_) : print_to_stderr_cb(print_to_stderr_cb_) {}

    PrintToStderrCB print_to_stderr_cb;
    int skip_frames = 1;
    int frame_idx = 0;
    bool has_first_line = false;

    void do_printf(const char* fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        int n = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        if (n > 0) {
            if (print_to_stderr_cb) {
                print_to_stderr_cb(buf, n);
            } else {
                eprintf("%s", buf);
            }
        }
    }

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

    static int on_bt_frame_static(void* self_, uintptr_t x, const char* filepath, int lineno, const char* c_funcname) {
        return ((PrintBacktrace*)self_)->on_bt_frame(x, filepath, lineno, c_funcname);
    }

    int on_bt_frame(uintptr_t x, const char* filepath, int lineno, const char* c_funcname) {
        if (frame_idx < skip_frames) {
            frame_idx += 1;
            return 0;
        }
        if (c_funcname && can_skip(c_funcname)) {
            if (frame_idx > 1) {
                frame_idx += 1;
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
            if (frame_idx > 1) {
                frame_idx += 1;
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
        if (!has_first_line) {
            first_line = true;
            has_first_line = true;
        }
        if (first_line) {
            do_printf("Location: " COLOR_PURPLE "%s" COLOR_NONE ":" COLOR_PURPLE "%d" COLOR_NONE "\n", filename, lineno);
            do_printf("\n  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ BACKTRACE ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
            do_printf("                               " COLOR_CYAN " ⋮ %d frames hidden ⋮" COLOR_NONE "\n", skip_frames);
        }

        // Skip the boring tail
        if (func_name && strcmp(func_name, "__libc_start_call_main") == 0) {
            skip_frames = INT_MAX;
        } else {
            // Print the frame
            char hyperlink_start[512];
            const char* hyperlink_end = "";
            hyperlink_start[0] = 0;
            if (get_hyperlink_url_root()) {
                const char* file_prefix = filepath[0] == '/' ? "" : "./";
                snprintf(hyperlink_start, sizeof(hyperlink_start), "\x1b]8;;%s%s%s:%d:5\a", get_hyperlink_url_root(), file_prefix, filepath,
                         lineno);
                hyperlink_end = "\x1b]8;;\a";
            }
            auto func_color = first_line ? COLOR_PINK : COLOR_GREEN;
            do_printf("% 4d: %s%s" COLOR_NONE
                      "\n"
                      "      at %s" COLOR_BBLUE "%s/" COLOR_BCYAN "%s" COLOR_NONE ":%d%s" COLOR_NONE "\n",
                      frame_idx, func_color, func_name, hyperlink_start, dir, filename, lineno, hyperlink_end);
            frame_idx += 1;
        }
        ::free(filename_);
        ::free(dir_);
        if (demangled) {
            ::free((void*)demangled);
        }
        return 0;
    }

    static void on_bt_error_static(void* self_, const char* msg, int errnum) {
        PrintBacktrace* self = (PrintBacktrace*)self_;
        self->do_printf("Error %d occurred when getting the stacktrace: %s", errnum, msg);
    }

    static const char* get_hyperlink_url_root() {
        // e.g. "vscode://vscode-remote/ssh-remote+newton"
        static const char* hyperlink_url_root = ::getenv("QBM_VSCODE_HYPERLINK_URL");
        return hyperlink_url_root;
    }
};

// ------------------------
// PrintBacktrace
//

// static
void CaptureBacktrace::init(const char* exec_filename) {
    backtrace_init(exec_filename);
    set_print_to_stderr_cb({});
}

// static
void CaptureBacktrace::set_print_to_stderr_cb(PrintToStderrCB print_to_stderr_cb_) {
    if (print_to_stderr_cb_) {
        print_to_stderr_cb = print_to_stderr_cb_;
    } else {
        print_to_stderr_cb_ = [](const char* msg, u32 len) { eprintf("%s", msg); };
    }
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
    PrintBacktrace p(print_to_stderr_cb);
    for (int i = 0; i < stack_len; ++i) {
        int rv = backtrace_pcinfo(__bt_state, stack[i], &PrintBacktrace::on_bt_frame_static, &PrintBacktrace::on_bt_error_static, &p);
    }
    p.do_printf("\n");
    if (p.frame_idx <= 1) {
        p.do_printf("CaptureBacktrace: no stack trace available\n");
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
    eprintf("Error %d occurred when initializing the stacktrace: %s", errnum, msg);
    bool* status = (bool*)data;
    *status = false;
}

bool backtrace_init(const char* filename) {
    bool status = true;
    __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, (void*)status);
    return status;
}

NOINLINE
void backtrace_print() {
    CaptureBacktrace bt{};
    bt.capture(1);
    bt.print();
}

// ---------------------------
