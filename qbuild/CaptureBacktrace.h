// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include <stdint.h>

/// NOTE: Must call CaptureBacktrace::init(argv[0]) in main()
struct CaptureBacktrace {
    CaptureBacktrace(int stack_max_ = 500);
    ~CaptureBacktrace();
    CaptureBacktrace(const CaptureBacktrace&) = delete;

    static void init(const char* exec_filename);

    void capture(int skip_frames = 0) const;
    void print() const;

   private:
    static int on_bt_frame(void* self_, uintptr_t pc);
    static void on_bt_error(void* self, const char* msg, int errnum);

    uint16_t stack_max{0};
    uint16_t stack_len{0};
    uintptr_t* stack;
};

char const* get_demangled_name(char const* const symbol) noexcept;
NOINLINE void backtrace_print();