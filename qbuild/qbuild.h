// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include "compiler.h"  // IWYU pragma: keep
#include "ctypes.h"    // IWYU pragma: keep
#include <errno.h>     // errno
#include <limits.h>    // INT_MAX
#include <stdint.h>
#include <string.h>  // strerror

//
// Forward declarations
//
namespace std {
template <typename _CharT>
struct char_traits;

template <typename _CharT, typename _Traits>
class basic_ostream;
typedef basic_ostream<char, char_traits<char>> ostream;
}  // namespace std

using namespace std;