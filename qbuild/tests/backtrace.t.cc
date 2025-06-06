// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#include "qbuild/CaptureBacktrace.h"
#include "qbuild/SigHandler.h"
#include "stdio.h"
#include <execinfo.h>

void unhandled_exception_handler();
volatile int g_val = 0;

__attribute__((noinline)) __attribute__((disable_tail_calls)) void foo2() {
    while (g_val == 0) {
    };
}
__attribute__((noinline)) __attribute__((disable_tail_calls)) __attribute__((optimize("no-optimize-sibling-calls"))) void foo() {
    foo2();
}

int main(int argc, char** argv) {
    SigHandler::install();

    foo();

    printf("Done\n");
    return 0;
}
