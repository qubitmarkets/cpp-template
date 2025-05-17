#include "qbuild/SigHandler.h"
#include "qbuild/compiler.h"
#include <stdarg.h>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <functional>

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "qbuild/Callback.h"

static int g_count = 0;

void testfn() {
    g_count = 1;
}

void invoke_callback(Callback<void()> cb) {
    cb();
}

void invoke_callback_ref(const Callback<void()>& cb) {
    cb();
}

CATCH_TEST_CASE("Callback") {
    Callback<void()> cbVoid;
    static_assert(sizeof(cbVoid) == 16);
    CATCH_CHECK(cbVoid.is_empty());

    // Static function
    g_count = 0;
    cbVoid = &testfn;
    cbVoid();
    CATCH_CHECK(g_count == 1);

    // no-capture lambda
    cbVoid = []() { g_count = 2; };
    cbVoid();
    CATCH_CHECK(g_count == 2);

    // ref-capture lambda
    int x = 1;
    CATCH_CHECK(x == 1);
    cbVoid = [&]() { x = 2; };
    CATCH_CHECK(!cbVoid.is_empty());

    // Run once
    cbVoid();
    CATCH_CHECK(x == 2);
    CATCH_CHECK(!cbVoid.is_empty());

    // Run again
    x = 3;
    cbVoid();
    CATCH_CHECK(x == 2);
    CATCH_CHECK(!cbVoid.is_empty());

    // Pass as a parameter and run
    x = 1;
    invoke_callback(cbVoid);
    CATCH_CHECK(x == 2);

    // Pass as a ref parameter and run
    x = 1;
    invoke_callback_ref(cbVoid);
    CATCH_CHECK(x == 2);
}

CATCH_TEST_CASE("Callback return int") {
    Callback<int(int)> cb = [](int) { return 2; };
    CATCH_CHECK(!cb.is_empty());
    CATCH_CHECK(cb(1) == 2);
}

CATCH_TEST_CASE("Callback init") {
    Callback<int(int)> cb;
    CATCH_CHECK(cb.is_empty());
    CATCH_CHECK(!cb);
    CATCH_CHECK((bool)cb == false);
}

CATCH_TEST_CASE("Callback noop") {
    {
        Callback<int(int)> cb;
        cb.set_noop();
        CATCH_CHECK(!cb.is_empty());
        CATCH_CHECK(!!cb);
        CATCH_CHECK((bool)cb);
        CATCH_CHECK(cb(1) == 0);
    }
    {
        Callback<int(int)> cb{Noop};
        CATCH_CHECK(!cb.is_empty());
        CATCH_CHECK(!!cb);
        CATCH_CHECK((bool)cb);
        CATCH_CHECK(cb(1) == 0);
    }
}

// Structures for testing member function pointers
struct AAA {
    int x = 0;
    void set(int v) { x = v; }
    virtual void setv(int v) { x = v; }
};
// This is the mangled C function name for the AAA::setv function
extern "C" void _ZN3AAA4setvEi(int v);

struct B : AAA {
    int b = 10;
};
struct Filler {
    int filler = 11;
};
struct C : Filler, AAA {
    int c = 20;
};
using set_cb_t = void (*)(AAA*, int);

// Accessor for the C function pointer from a member function pointer
template <typename T, typename MemFn>
set_cb_t get_fptr(T MemFn::* memfn) {
    auto raw = (u64*)&memfn;
    auto p1 = raw[0];
    CATCH_REQUIRE(raw[1] == 0);
    if (p1 < 4096) {
        CATCH_REQUIRE(p1 == 1);  // First virtual function table entry
        // virtual member function
        auto cb = [](AAA* a, int v) {
            auto vtable = *(u64**)a;
            auto fptr = (set_cb_t)vtable[0];
            CATCH_REQUIRE((u64)_ZN3AAA4setvEi == (u64)fptr);
            fptr(a, v);
        };
        return (set_cb_t)cb;
    } else {
        return (set_cb_t)p1;
    }
}

// Support function for testing C function pointers
static int g_test_value = 0;
void set_test_value(int v) {
    g_test_value = v;
}

const char* stringf(const char* fmt, ...) {
    static int i = 0;
    i = (i + 1) % 4;
    static char buf[4][1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf[i], sizeof(buf), fmt, args);
    va_end(args);
    return buf[i];
}

CATCH_TEST_CASE("Callback raw") {
    static u64 base = 0;
    AAA a;
    B b;

    C c;
    auto memfn = &AAA::set;
    struct MemFnPointer {
        u64 s1;
        u64 s2;
    };
    static_assert(sizeof(void*) == 8);
    static_assert(sizeof(memfn) == 16);
    static_assert(sizeof(MemFnPointer) == 16);

    {
        CATCH_INFO("Base == &a");
        base = (u64)&a;
        auto* memfn_raw = reinterpret_cast<MemFnPointer*>(&memfn);
        void** vtable = *(void***)(&a);
        auto* vtable_raw = (MemFnPointer*)vtable;
        CATCH_INFO(stringf("cbraw_a: %016lx %016lx\n", memfn_raw->s1, memfn_raw->s2));
        CATCH_INFO(stringf("vtable[0]: %016lx [1]:%016lx\n", (u64)vtable[0], (u64)vtable[1]));
        CATCH_INFO(stringf("vtable_raw: %016lx %016lx\n", vtable_raw->s1, vtable_raw->s2));
        static_assert(__is_member_function_pointer(decltype(&AAA::set)) == true);
        auto set_cb = get_fptr(&AAA::set);
        CATCH_CHECK(a.x == 0);
        set_cb(&a, 1);
        CATCH_CHECK(a.x == 1);
        set_cb = get_fptr(&AAA::setv);
        set_cb(&a, 2);
        CATCH_CHECK(a.x == 2);

        auto set_cb2 = makeCallback(&a, &AAA::set);
        a.x = 0;
        set_cb2(1);
        CATCH_CHECK(a.x == 1);
        // Check == operator
        CATCH_CHECK(set_cb2 == set_cb2);

        auto set_cb3 = Callback<void(int)>(&a, &AAA::setv);
        set_cb3(2);
        CATCH_CHECK(a.x == 2);
        CATCH_CHECK(set_cb3.func == (void*)&_ZN3AAA4setvEi);
        CATCH_CHECK(set_cb3.data == set_cb2.data);
    }

    {
        CATCH_INFO("Base == &b");
        base = (u64)&b;
        auto memfn_b = &B::set;
        auto* memfn_raw = reinterpret_cast<MemFnPointer*>(&memfn_b);
        CATCH_INFO(stringf("cbraw b: %016lx %016lx\n", memfn_raw->s1, memfn_raw->s2));
        auto set_cb = get_fptr(&B::set);
        CATCH_CHECK(b.x == 0);
        b.set(1);
        CATCH_CHECK(b.x == 1);
        set_cb(&b, 2);
        CATCH_CHECK(b.x == 2);
        set_cb = get_fptr(&B::setv);
        set_cb(&b, 3);
        CATCH_CHECK(b.x == 3);

        auto set_cb2 = makeCallback(&b, &B::set);
        b.x = 0;
        set_cb2(1);
        CATCH_CHECK(b.x == 1);
        set_cb2 = Callback<void(int)>(&b, &B::setv);
        set_cb2(2);
        CATCH_CHECK(b.x == 2);
    }

    // C has multiple inheritance, AAA is offset from this
    {
        CATCH_INFO("Base == &c");
        base = (u64)&c;
        auto memfn_c = &C::set;
        auto* memfn_raw = reinterpret_cast<MemFnPointer*>(&memfn_c);
        CATCH_INFO(stringf("cbraw c: %016lx %016lx\n", memfn_raw->s1, memfn_raw->s2));
        auto set_cb = get_fptr(&C::set);
        CATCH_CHECK(c.x == 0);
        // When we call c.set, the compiler first offsets the pointer to AAA
        c.set(1);
        CATCH_CHECK(c.x == 1);
        // When we take the address of &AAA from &C, the compiler adds the offset
        set_cb(&c, 2);
        CATCH_CHECK(c.x == 2);
        set_cb = get_fptr(&C::setv);
        set_cb(&c, 3);
        CATCH_CHECK(c.x == 3);

        // auto set_cb2 = Callback<AAA, void(int)>(c, &C::set);
        auto set_cb2 = makeCallback(&c, &C::set);
        c.x = 0;
        set_cb2(1);
        CATCH_CHECK(c.x == 1);
        set_cb2 = Callback<void(int)>(&c, &C::setv);
        set_cb2(2);
        CATCH_CHECK(c.x == 2);
        CATCH_CHECK(set_cb2.func == (void*)&_ZN3AAA4setvEi);
    }

    // C function pointer
    {
        auto set_cb = &set_test_value;
        g_test_value = 0;
        CATCH_CHECK(g_test_value == 0);
        set_test_value(1);
        CATCH_CHECK(g_test_value == 1);
        set_cb(2);
        CATCH_CHECK(g_test_value == 2);
    }
}

void add_v_cfunc(u32& x, int i) {
    for (int j = 0; j < i % 10; ++i)
        x += i;
}

ALWAYS_INLINE u64 get_now() {
    timespec tp;
    ::clock_gettime(CLOCK_MONOTONIC, &tp);
    u64 nanos = tp.tv_sec * 1'000'000'000 + tp.tv_nsec;
    return nanos;
}

CATCH_TEST_CASE("Callback perf test") {
    auto warmup_iters = 1'000'000;
    // On my laptop, Callback completes in 34ms, std::function in 122ms
    auto iters = 10'000'000;
    u32 x = 0;

    // Run an std::function and a Callback labmda to compare performance
    // std::function
    std::function<void(u32 & x, int)> add_stdfunc = [&](u32& x, int i) {
        for (int j = 0; j < i % 10; ++i)
            x += i;
    };
    void (*cfunc)(u32&, int) = &add_v_cfunc;
    if (rand() == 575743)  // prevent compiler from inlining as an optimization
        cfunc = nullptr;
    auto run_stdfunc = [&](int iters) {
        x = 0;
        for (int i = 0; i < iters; ++i) {
            add_stdfunc(x, i);
        }
    };
    run_stdfunc(warmup_iters);
    auto start = get_now();
    run_stdfunc(iters);
    auto std_function_dur = get_now() - start;
    CATCH_CHECK(x == 3903231744);
    CATCH_CHECK(std_function_dur <= 300'000'000);  // 300ms

    // Lambda callback
    Callback<void(u32 & x, int)> add_lambdacb = [&](u32& x, int i) {
        for (int j = 0; j < i % 10; ++i)
            x += i;
    };
    auto run_cblambda = [&](int iters) {
        x = 0;
        for (int i = 0; i < iters; ++i) {
            add_lambdacb(x, i);
        }
    };
    run_cblambda(warmup_iters);
    start = get_now();
    run_cblambda(iters);
    auto cb_dur = get_now() - start;
    CATCH_CHECK(x == 3903231744);
    CATCH_CHECK(cb_dur <= 200'000'000);  // 200ms
    CATCH_CHECK(cb_dur + 1'000'000 <= std_function_dur);

    // Member function callback
    struct MyStuct {
        void add(int i) {
            for (int j = 0; j < i % 10; ++i)
                x += i;
        }
        u32 x = 0;
    };
    MyStuct my_struct;
    auto run_cbmemfn = [&](int iters) {
        my_struct.x = 0;
        for (int i = 0; i < iters; ++i) {
            my_struct.add(i);
        }
    };
    run_cbmemfn(warmup_iters);
    start = get_now();
    run_cbmemfn(iters);
    auto cb_memfn_dur = get_now() - start;
    CATCH_CHECK(my_struct.x == 3903231744);
    CATCH_CHECK(cb_memfn_dur <= 200'000'000);  // 200ms
    CATCH_CHECK(cb_memfn_dur + 1'000'000 <= std_function_dur);
    CATCH_CHECK(cb_memfn_dur + 1'000'000 <= cb_dur);
}
