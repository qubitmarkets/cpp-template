// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include <stdint.h>
#include <unistd.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i128 = __int128_t;

ALWAYS_INLINE constexpr u64 u128_lower(u128 v) {
    return (u64)v;
}
ALWAYS_INLINE constexpr u64 u128_upper(u128 v) {
    return (u64)(v >> 64L);
}

struct alignas(32) u256 {
    constexpr u256() noexcept = default;
    constexpr u256(u64 v0, u64 v1, u64 v2, u64 v3) noexcept : _v{v0, v1, v2, v3} {}
    u256(unsigned char v[32]) noexcept {
        u64* v64 = (u64*)v;
        _v[0] = v64[0];
        _v[1] = v64[1];
        _v[2] = v64[2];
        _v[3] = v64[3];
    }

    constexpr bool operator==(const u256& other) const noexcept {
        return _v[0] == other._v[0] && _v[1] == other._v[1] && _v[2] == other._v[2] && _v[3] == other._v[3];
    }

    u64 _v[4]{};
};
