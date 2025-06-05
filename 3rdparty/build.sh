#!/bin/bash

set -eu

cd $(dirname ${BASH_SOURCE})
export INFRA_ROOT=/opt/infra.1/
export PATH=$PATH:$INFRA_ROOT/bin

export CLANG_MAJOR_VER=20
export GCC_MAJOR_VER=15

export profile=release
export compiler=gcc14
export install_root=$PWD
source ./toolchain.sh

# Build the build tools
if [[ ! -e $INFRA_ROOT/bin/cmake ]]; then
    ./build_cmake.sh
fi

# Build the compilers
GCC_VER=$(gcc --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
if [[ $GCC_VER -lt ${GCC_MAJOR_VER} ]]; then
    GCC_VER=$(/opt/infra.1/bin/gcc-${GCC_MAJOR_VER} --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
    if [[ $GCC_VER -lt ${GCC_MAJOR_VER} ]]; then
        echo "Build GCC"
        ./build_gcc.sh
    fi
fi
if [[ $(clang++-$CLANG_MAJOR_VER --version | head -1 | grep -c "version $CLANG_MAJOR_VER\.") -eq 0 ]]; then
    echo "Build Clang"
    ./build_clang.sh $CLANG_MAJOR_VER
fi

if [[ ! -e f-stack/lib ]]; then
    ./build_fstack.sh
fi

for compiler in gcc15 clang20; do
    for dot_profile_extras in "" ".sanitize"; do
        export compiler
        export dot_profile_extras
        export profile=release${dot_profile_extras}
        export build_dir=builds/$profile.$compiler
        export install_dir=$install_root/$profile.$compiler
        source ./toolchain.sh
        echo "compiler=$compiler"
        echo "profile=$profile"
        echo "install_dir=$install_dir"

        if [[ ! -e $install_dir/lib/libbacktrace.a ]]; then
            ./build_libbacktrace.sh
        fi
        if [[ ! -e $install_dir/lib/libCatch2.a ]]; then
            ./build_catch2.sh
        fi
        if [[ ! -e $install_dir/lib/libabsl_base.a ]]; then
            ./build_abseil.sh
        fi
        if [[ ! -e $install_dir/lib/libwslay.a ]]; then
            ./build_wslay.sh
        fi
        if [[ ! -e $install_dir/lib/libsimdjson_static.a ]]; then
            ./build_simdjson.sh
        fi
    done
done
