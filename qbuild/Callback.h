// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

/// Fast Callback
/// Callback with 8 bytes of storage
///
/// Lifetimes:
/// - Callback has a lifetime L1
/// - The item it's calling to has a lifetime L2
/// - If L2 < L1, then calling the CB will segfault.
///
/// NOTE: Member functions will be devirtualized at the time of assignment to the Callback.
///
/// How does it work?
/// - C++ member function calls are, in Linux, implemented in the same way as a C function call with the this pointer as the first argument.
/// - In this case, the Callback stores the "this" pointer in the data field, and a function pointer in the func field
/// - When the member function is non-virtual, the function pointer is the address of the member function.
/// - When the member function is virtual, the function pointer is devirtualized.
///
/// - For Lambda functions, it creates a helper class with a static function that calls the lambda with the captured data.
/// - The Callback's func member points to this static function
/// - The Callback's data member is used to store the captured data, which is copied into the Callback at the time of assignment.
///
/// - For C functions, the Callback's func member points to a stub C function, which removes the first parameter (there is no "this"). The
/// data member is the C function pointer itself.
///

#include "qbuild/compiler.h"
#include "qbuild/ctypes.h"
#include <stdlib.h>  // ::abort
#include <concepts>
#include <type_traits>
#include <utility>  // std::forward

// Placeholder Tag struct for initializing Callback to be callable with no side-effects
struct __Tag_Noop {};
static const constexpr __Tag_Noop Noop;

struct CallbackStorage {
    CallbackStorage() = default;
    CallbackStorage(void* func_) : func(func_) {}

    bool operator==(const CallbackStorage& other) const { return func == other.func && data == other.data; }

    // Accessors
    bool is_empty() const { return !func; }
    explicit operator bool() const { return !func; }

    void reset() {
        func = nullptr;
        data = nullptr;
    }

    void* func{nullptr};
    void* data{};
};

// 8 bytes for function pointer, 8 bytes for storage/this pointer.
// If it's <=16 bytes, it'll be passed in registers
template <typename Result, typename... Args>
class Callback;

template <typename Functor, typename FuncSig>
concept IsCallback = std::same_as<std::decay_t<Functor>, Callback<FuncSig>>;

template <typename Result, typename... Args>
struct Callback<Result(Args...)> : CallbackStorage {
    using FuncSig = Result(Args...);
    using InvokeSig = Result(void*, Args...);

    template <typename Functor>
    struct LambdaHelper {
        static_assert(std::is_invocable_v<Functor, Args...>, "Lambda must be invocable with Args");
        static_assert(std::is_convertible_v<decltype(std::declval<Functor>()(std::declval<Args>()...)), Result>,
                      "Lambda must return Result type");

        static Result call(void* data, Args... args) { return (*reinterpret_cast<Functor*>(&data))(static_cast<Args&&>(args)...); }
    };

    Callback() noexcept {}
    Callback(const Callback&) noexcept = default;
    Callback(Callback&&) noexcept = default;
    Callback& operator=(const Callback&) noexcept = default;

    // initialize with noop
    Callback(__Tag_Noop) noexcept : CallbackStorage() { set_noop(); }

    // Delete all other constructors that would allow copying or moving from a different signature
    template <typename TResult, typename... TArgs>
    Callback(const Callback<TResult(TArgs...)>&) = delete;
    template <typename TResult, typename... TArgs>
    Callback(Callback<TResult(TArgs...)>&&) = delete;

    // ---------------------
    // C Functions
    //
    template <typename... _Args>
    static Result stub_func(void* fptr, _Args... args) {
        return (*reinterpret_cast<Result (*)(_Args...)>(fptr))(args...);
    }

    template <typename _Result, typename... _Args>
    Callback(_Result (*f)(_Args...)) {
        func = (void*)&stub_func<_Args...>;
        data = (void*)f;
    }

    // --------------------
    // Lambda Capture
    //
    template <typename Functor>
        requires(
            !IsCallback<Functor, FuncSig> && std::is_invocable_v<Functor, Args...> && std::is_trivially_destructible_v<Functor> &&
            std::is_convertible_v<decltype(std::declval<Functor>()(std::declval<Args>()...)), Result>)  // Lambda must return Result type
    Callback(Functor&& f) {
        static_assert(sizeof(f) <= 8, "Lambda capture too large");
        func = (void*)&LambdaHelper<std::decay_t<Functor>>::call;
        data = 0;
        memcpy(&data, &f, sizeof(Functor));
    }

    // --------------------
    // Member Function
    //
    template <typename _T, typename T, typename MemFn>
        requires(std::is_member_function_pointer_v<MemFn T::*> && std::is_base_of_v<T, _T>)
    Callback(_T* t, MemFn T::* memfn) noexcept {
        auto raw = (u64*)&memfn;
        auto p1 = raw[0];
        if (p1 < 4096) {
            // Virtual member function. Devirtualize.
            auto vtable = *(u64**)t;
            auto fptr = vtable[(p1 - 1) / 8];
            func = (void*)fptr;
        } else {
            // Non-virtual member function
            func = (void*)p1;
        }
        data = (void*)t;
    }

    // Call
    template <typename... _Args>
        requires(... && std::is_convertible_v<_Args, Args>)
    ALWAYS_INLINE Result call(Args&&... args) const {  //
        return (*reinterpret_cast<InvokeSig*>(func))(data, static_cast<Args>(args)...);
    }
    template <typename... _Args>
        requires(... && std::is_convertible_v<_Args, Args>)
    ALWAYS_INLINE Result operator()(_Args&&... args) const {
#if !defined(NDEBUG)
        if (!func) {
            ::abort();
        }
#endif
        return (*reinterpret_cast<InvokeSig*>(func))(data, static_cast<Args>(args)...);
    }

    // Accessors
    bool is_empty() const { return !func; }
    explicit operator bool() const { return func; }

    // Comparison
    bool operator==(const Callback& other) const { return func == other.func && data == other.data; }

    // Modifies
    void set_noop() { func = (void*)&noop; }
    static Result noop(void*, Args&&...) { return Result(); }
};
static_assert(sizeof(Callback<void()>) == 16, "Must be 16 bytes to fit in 2 registers");

// Makes a callback to a member function
// If the member function is virtual, it will be devirtualized
template <typename _TClass, typename TClass, typename MemFn>
    requires(std::is_member_function_pointer_v<MemFn TClass::*> && std::is_base_of_v<TClass, _TClass>)
Callback<MemFn> makeCallback(_TClass* t, MemFn TClass::* memfn) {
    return Callback<MemFn>(t, memfn);
}

// Makes a callback to a lambda
template <typename Lambda, typename Result, typename... Args>
    requires(std::is_invocable_v<Lambda, Args...> && std::is_same_v<decltype(std::declval<Lambda>()(std::declval<Args>()...)), Result>)
Callback<Result(Args...)> makeCallback(Lambda&& f) {
    return Callback<Result(Args...)>(std::forward<Lambda>(f));
}

// Makes a callback to a C function
template <typename Result, typename... Args>
Callback<Result(Args...)> makeCallback(Result (*cfunc)(Args...)) {
    return Callback<Result(Args...)>(cfunc);
}
