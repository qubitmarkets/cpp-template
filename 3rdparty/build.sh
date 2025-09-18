#!/bin/bash

set -eu

cd $(dirname ${BASH_SOURCE})
export INFRA_ROOT=/opt/infra.1/
export PATH=$PATH:$INFRA_ROOT/bin

export CLANG_MAJOR_VER=21
export GCC_MAJOR_VER=15

# Save these as we reset them in toolchain.sh
export CFLAGS_ORIG="${CFLAGS-}"
export LDFLAGS_ORIG="${LDFLAGS-}"

export profile=release
export compiler=gcc${GCC_MAJOR_VER}
export install_root=$PWD
source ./toolchain.sh

# Force rebuild of a library by calling ./build.sh <library_name>
target="${1-}"

# Build the compilers
GCC_VER=$(gcc --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
if [[ $GCC_VER -lt ${GCC_MAJOR_VER} || $target == "gcc" ]]; then
    GCC_VER=$(/opt/infra.1/bin/gcc-${GCC_MAJOR_VER} --version | head -1 | cut -d' ' -f3 | cut -d. -f1)
    if [[ $GCC_VER -lt ${GCC_MAJOR_VER} ]]; then
        echo "Build GCC"
        ./build_gcc.sh
    fi
fi
# Build cmake (requires g++-15)
if [[ ! -e $INFRA_ROOT/bin/cmake || $target == "cmake" ]]; then
    ./build_cmake.sh
fi

if [[ $(clang++-$CLANG_MAJOR_VER --version | head -1 | grep -c "version $CLANG_MAJOR_VER\.") -eq 0 || $target == "clang" ]]; then
    echo "Build Clang"
    ./build_clang.sh $CLANG_MAJOR_VER
fi

if [[ ! -e f-stack/lib || $target == "fstack" ]]; then
    ./build_fstack.sh
fi

for compiler in gcc${GCC_MAJOR_VER} clang${CLANG_MAJOR_VER}; do
    for dot_profile_extras in "" ".sanitize"; do
        export compiler
        export dot_profile_extras
        export profile=release${dot_profile_extras}
        export build_dir=builds/$profile.$compiler
        export install_dir=$install_root/$profile.$compiler
        source ./toolchain.sh
        echo "---------------------------"
        echo "compiler=$compiler"
        echo "profile=$profile"
        echo "install_dir=$install_dir"
        echo "CFLAGS=${CFLAGS-}"
        echo "LDFLAGS=${LDFLAGS-}"
        echo "---------------------------"
        #read -p "press enter"

        if [[ ! -e $install_dir/lib64/libssl.a || $target == "openssl" ]]; then
            #./build_openssl.sh
true
        fi
        if [[ ! -e $install_dir/lib/libnuma.a || $target == "numactl" ]]; then
            ./build_numactl.sh
        fi
        if [[ ! -e $install_dir/lib/libbacktrace.a || $target == "libbacktrace" ]]; then
            ./build_libbacktrace.sh
        fi
        if [[ ! -e $install_dir/lib/libCatch2.a || $target == "catch2" ]]; then
            ./build_catch2.sh
        fi
        if [[ ! -e $install_dir/lib/libabsl_base.a || $target == "abseil" ]]; then
            ./build_abseil.sh
        fi
        if [[ ! -e $install_dir/lib/libwslay.a || $target == "wslay" ]]; then
            ./build_wslay.sh
        fi
        if [[ ! -e $install_dir/lib/libsimdjson_static.a || $target == "simdjson" ]]; then
            ./build_simdjson.sh
        fi
        echo "---------------------------"
        echo "Install done: $install_dir"
        echo "---------------------------"
    done
done
