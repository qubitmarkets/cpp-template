// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#include "qbuild/CaptureBacktrace.h"
#include "qbuild/SigHandler.h"
#include "stdio.h"
#include <execinfo.h>
#include <ctime>

void unhandled_exception_handler();
volatile int g_val = 0;

//
// Use a macro so the backtrace is always on one line
//
#define CHECK_TIME()                        \
    ::clock_gettime(CLOCK_REALTIME, &tp);   \
    if (tp.tv_sec - start_tp.tv_sec > 10) { \
        return;                             \
    }

//
__attribute__((noinline)) __attribute__((disable_tail_calls)) void foo2() {
    struct timespec tp;
    struct timespec start_tp;
    ::clock_gettime(CLOCK_REALTIME, &start_tp);
    while (g_val == 0) {
        CHECK_TIME()
    };
}
__attribute__((noinline)) __attribute__((disable_tail_calls)) __attribute__((optimize("no-optimize-sibling-calls"))) void foo() {
    foo2();
}

int main(int argc, char** argv) {
    SigHandler::install();

    foo();

    return 0;
}
