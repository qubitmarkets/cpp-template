// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

// Current Build Profile (can only be one)
#define BUILD_PROFILE_DEBUG 1
#define BUILD_PROFILE_RELEASE 2
#if defined(NDEBUG)
    #define CURRENT_BUILD_PROFILE BUILD_PROFILE_RELEASE
#else
    #define CURRENT_BUILD_PROFILE BUILD_PROFILE_DEBUG
#endif

// Build configuration.  Any or none.
#ifndef BUILD_CFG_TESTING
    #define BUILD_CFG_TESTING 0
#endif

// Compiler detection
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

// Compiler defines
#define NOINLINE __attribute__((noinline))

// INLINE, ALWAYS_INLINE
// If optimizations are disabled, forcing inlining can lead to significant
// code bloat and high compile times. Don't use simdjson_really_inline for
#if CURRENT_BUILD_PROFILE == BUILD_PROFILE_DEBUG
    #if __QBUILD_COMPILER_CLANG__
        #define INLINE __attribute__((weak))
        #define ALWAYS_INLINE __attribute__((weak))
    #else
        #define INLINE inline
        #define ALWAYS_INLINE inline
    #endif
#else
    #define INLINE __attribute__((inline))
    #define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif

// A function attribute.  Indicates that a return pointer can assumed bo be non-null
// Put this at the start of the function declaration
// Or, use NOT_NULL(1,2) to indicate tat the first and second arguments are non-null
#define NOT_NULL __attribute__((nonnull))
#define WEAK __attribute__((weak))

// Macro support
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
// 1) Helper to pick the 64th parameter out of a long list:
#define _COUNT_ARGS_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, \
                      _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48,  \
                      _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...)                                  \
    N

// 2) Reverse sequence from 63 down to 0:
#define _COUNT_ARGS_RSEQ_N()                                                                                                            \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
        30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// 3) Glue __VA_ARGS__ together with the reverse sequence, then extract “N”:
#define _COUNT_ARGS(...) _COUNT_ARGS_N(__VA_ARGS__)
#define COUNT_ARGS(...) _COUNT_ARGS(__VA_ARGS__, _COUNT_ARGS_RSEQ_N())

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

// FAR
struct NoFarReturn {};

// Put this code in a far-away memory page
// Ref : https://sourceware.org/binutils/docs/as/Section.html
// Note : Need the "mutable" keyword to avoid compiler making half of them const .sections
// TODO: gcc still has a problem. https://stackoverflow.com/questions/35091862/inline-static-data-causes-a-section-type-conflict
#define FAR(code)                                                                                          \
    {                                                                                                      \
        [&]() __attribute__((noinline)) __attribute__((section(".text_far,\"ax\",@progbits#execinstr"))) { \
            code;                                                                                          \
            return ::bb::NoFarReturn{};                                                                    \
        }();                                                                                               \
    }

// Same as kernel definition
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

// simdjson
#define SIMDJSON_EXCEPTIONS 1

// Catch2 disable signal handler, wider INFO
#define CATCH_CONFIG_NO_POSIX_SIGNALS
#define CATCH_CONFIG_CONSOLE_WIDTH 180

// Backtrace
extern void backtrace_print();
