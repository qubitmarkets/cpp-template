// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include "qbuild/Callback.h"
#include <stdint.h>

using PrintToStderrCB = Callback<void(const char* line, u32 len)>;

/// NOTE: Must call CaptureBacktrace::init(argv[0]) in main()
struct CaptureBacktrace {
    CaptureBacktrace(int stack_max_ = 500);
    ~CaptureBacktrace();
    CaptureBacktrace(const CaptureBacktrace&) = delete;

    static void init(const char* exec_filename);
    static void set_print_to_stderr_cb(PrintToStderrCB print_to_stderr_cb);

    void capture(int skip_frames = 0) const;
    void print() const;

   private:
    static int on_bt_frame(void* self_, uintptr_t pc);
    static void on_bt_error(void* self, const char* msg, int errnum);

    uint16_t stack_max{0};
    uint16_t stack_len{0};
    uintptr_t* stack;
    static PrintToStderrCB print_to_stderr_cb;
};

char const* get_demangled_name(char const* const symbol) noexcept;
NOINLINE void backtrace_print();