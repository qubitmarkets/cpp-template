#!/bin/bash

set -eu

cd $(dirname ${BASH_SOURCE})
PATH=$PATH:/opt/infra.1/bin

GCC_MAJOR_VER=14
CLANG_MAJOR_VER=19

GCC_VER=$(gcc --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
if [[ $GCC_VER -lt ${GCC_MAJOR_VER} ]]; then
    GCC_VER=$(/opt/infra.1/bin/gcc-14 --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
    if [[ $GCC_VER -lt ${GCC_MAJOR_VER} ]]; then
        echo "Build GCC"
        ./build_gcc.sh
    fi
fi
if [[ $(clang++-$CLANG_MAJOR_VER --version | head -1 | grep -c "version $CLANG_MAJOR_VER\.") -eq 0 ]]; then
    echo "Build Clang"
    ./build_clang.sh $CLANG_MAJOR_VER
fi
if [[ ! -e lib/libbacktrace.a ]]; then
    ./build_libbacktrace.sh
fi
if [[ ! -e lib/libCatch2.a ]]; then
    ./build_catch2.sh
fi
if [[ ! -e lib/libabsl_base.a ]]; then
    ./build_abseil.sh
fi
if [[ $(cmake --version | grep -c "4\.0") -eq 0 ]]; then
    ./build_cmake.sh
fi
if [[ ! -e f-stack/lib ]]; then
    ./build_fstack.sh
fi
