#pragma once

#define NOINLINE __attribute__((noinline))
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
#define INLINE __attribute__((inline))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define WEAK __attribute__((weak))
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

// Same as kernel definition
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

// For catch2 tests
#define CATCH_CONFIG_PREFIX_ALL

#if defined(__clang__)
    #define __QBUILD_COMPILER__ "clang"
    #define __QBUILD_COMPILER_VERSION__ __clang_major__
#elif defined(__GNUC__)
    #define __QBUILD_COMPILER__ "gcc"
    #define __QBUILD_COMPILER_VERSION__ __GNUC__
#else
    #error "Unsupported compiler"
#endif
