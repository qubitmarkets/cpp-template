// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include <stdint.h>
#include <unistd.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

struct alignas(16) u128 {
    u128(unsigned char v[32]) {
        u64* v64 = (u64*)v;
        _v[0] = v64[0];
        _v[1] = v64[1];
    }

    u64 _v[2];
};

struct alignas(32) u256 {
    u256(unsigned char v[32]) {
        u64* v64 = (u64*)v;
        _v[0] = v64[0];
        _v[1] = v64[1];
        _v[2] = v64[2];
        _v[3] = v64[3];
    }

    u64 _v[4];
};