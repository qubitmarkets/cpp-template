// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#define NOINLINE __attribute__((noinline))
#define INLINE __attribute__((inline))
// A function attribute.  Indicates that a return pointer can assumed bo be non-null
// Put this at the start of the function declaration
// Or, use NOT_NULL(1,2) to indicate tat the first and second arguments are non-null
#define NOT_NULL __attribute__((nonnull))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define WEAK __attribute__((weak))
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

// COLD is used to mark functions that are not expected to be called frequently
// It moves the function code to a separate section, and the compiler will not inline it
#define COLD __attribute__((cold))
#define COLD_START() \
    {                \
        [[unlikely]][&]() __attribute__((cold)) __attribute__((noinline)) {
#define COLD_END() \
    }              \
    ();            \
    }
#define COLD_LAMBDA(code) \
    COLD_START() code;    \
    COLD_END()

#define IF_COLD(cond, code)    \
    if unlikely (cond) {       \
        COLD_LAMBDA_START()    \
        code COLD_LAMBDA_END() \
    }

// Same as kernel definition
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

// For catch2 tests
#define CATCH_CONFIG_PREFIX_ALL

#if defined(__clang__)
    #define __QBUILD_COMPILER_GCC__ 0
    #define __QBUILD_COMPILER_CLANG__ 1
    #define __QBUILD_COMPILER_VERSION__ __clang_major__
#elif defined(__GNUC__)
    #define __QBUILD_COMPILER_GCC__ 1
    #define __QBUILD_COMPILER_CLANG__ 0
    #define __QBUILD_COMPILER_VERSION__ __GNUC__
#else
    #error "Unsupported compiler"
#endif
