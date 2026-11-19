// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include "Callback.h"
#include <stddef.h>
#include <stdint.h>

using PrintToStderrCB = Callback<void(const char* line, u32 len)>;

/// NOTE: Must call CaptureBacktrace::init(argv[0]) in main()
struct CaptureBacktrace {
    CaptureBacktrace(int stack_max_ = 500);
    ~CaptureBacktrace();
    CaptureBacktrace(const CaptureBacktrace&) = delete;

    static void init(const char* exec_filename);
    static void set_print_to_stderr_cb(PrintToStderrCB print_to_stderr_cb);
    static PrintToStderrCB get_print_to_stderr_cb() { return print_to_stderr_cb; }

    /// The checkout that debug info's /vcache/src/... paths name: $QBM_SOURCE_ROOT, else CMAKE_HOME_DIRECTORY from the
    /// nearest CMakeCache.txt above the executable. nullptr if neither is found. Resolved once, by init().
    static const char* source_root();
    /// filepath with a leading /vcache/src replaced by source_root(), written to buf; filepath itself if it has no
    /// such prefix, or there is no source root, or buf is too small.
    static const char* map_source_path(const char* filepath, char* buf, size_t buf_size);

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

NOINLINE void backtrace_print();