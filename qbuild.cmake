cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)
project(qbuild
    VERSION 1.0
    DESCRIPTION "qbuild"
    LANGUAGES CXX)
find_package(Catch2 3 REQUIRED PATHS ${THIRD_PARTY_DIR}/lib/cmake)

# Setup Build environment
execute_process(COMMAND "/usr/bin/ln" "-nfs" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/build")

# 3rd party reqs
include(${THIRD_PARTY_DIR}/lib/cmake/libbacktrace.cmake)
include(CTest)
include(Catch)
find_package(OpenSSL REQUIRED)

include_directories("${THIRD_PARTY_DIR}/include")
include_directories(".")
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=bugprone-*,clang-analyzer-*,performance-*")
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

# CMake options
option(CLANG_TIME_TRACE "Enable clang profiling." OFF)
if(CLANG_TIME_TRACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
endif()

# qbuild
set(qbuild_src
    qbuild/CaptureBacktrace.cc
    qbuild/SigHandler.cc
)

add_library(qbuild ${qbuild_src})
target_link_directories(qbuild PUBLIC "${THIRD_PARTY_DIR}/lib")
target_link_libraries(qbuild -lbacktrace)
#target_precompile_headers(qbuild PUBLIC qbuild/pch.h)
set_target_properties(qbuild
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# ----------------------
# Binaries
#


# ----------------------
# Tests
#
option(QBUILD_TESTS "Enable qbuild tests" OFF)
if(QBUILD_TESTS)
    add_executable(backtrace.t qbuild/tests/backtrace.t.cc)
    target_compile_options(backtrace.t PRIVATE -Wno-error=attribute-warning -Wno-unknown-attributes -Wno-attributes)
    target_link_libraries(backtrace.t PRIVATE qbuild)

    enable_testing()
    add_test(NAME backtrace.t  COMMAND 
        ${CMAKE_CURRENT_SOURCE_DIR}/qbuild/tests/backtrace.sh 
        $<TARGET_FILE:backtrace.t>)


    add_executable(callback.t 
        qbuild/tests/_run_catch_tests.t.cc
        qbuild/tests/Callback.t.cc
    )
    target_link_libraries(callback.t PRIVATE qbuild)
    target_link_libraries(callback.t PRIVATE Catch2::Catch2)
    catch_discover_tests(callback.t)
endif()
unset(QBUILD_TESTS CACHE)