// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#include "qbuild/CaptureBacktrace.h"
#include <limits.h>
#include <unistd.h>
#include <string>

#include <catch2/catch_test_macros.hpp>

// The test binary is in <checkout>/builds/<preset>/, so the source root is found from CMakeCache.txt. No path is
// compiled in: that would make this object differ per checkout.
TEST_CASE("source_root is the checkout this binary was built from") {
    const char* root = CaptureBacktrace::source_root();
    REQUIRE(root != nullptr);
    CHECK(::access((std::string(root) + "/qbuild/CaptureBacktrace.cc").c_str(), R_OK) == 0);
}

TEST_CASE("map_source_path maps /vcache/src to the source root") {
    const char* root = CaptureBacktrace::source_root();
    REQUIRE(root != nullptr);
    char buf[PATH_MAX];

    CHECK(std::string(CaptureBacktrace::map_source_path("/vcache/src/qbuild/CaptureBacktrace.cc", buf, sizeof(buf))) ==
          std::string(root) + "/qbuild/CaptureBacktrace.cc");
    CHECK(std::string(CaptureBacktrace::map_source_path("/vcache/src", buf, sizeof(buf))) == root);

    // Not the /vcache/src prefix: returned as is
    for (const char* path : {"/vcache/srcx/a.cc", "/vcache/other/a.cc", "/usr/include/stdio.h", "rel/a.cc", ""}) {
        CHECK(CaptureBacktrace::map_source_path(path, buf, sizeof(buf)) == path);
    }
    CHECK(CaptureBacktrace::map_source_path(nullptr, buf, sizeof(buf)) == nullptr);

    // Too long for the buffer: returned as is, not truncated
    const char* path = "/vcache/src/qbuild/CaptureBacktrace.cc";
    char small[8];
    CHECK(CaptureBacktrace::map_source_path(path, small, sizeof(small)) == path);
}
