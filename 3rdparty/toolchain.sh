#/bin/bash

export compiler=${compiler-gcc}
GCC_MAJOR_VER=${GCC_MAJOR_VER-14}
CLANG_MAJOR_VER=${CLANG_MAJOR_VER-19}

if [[ $compiler == gcc ]]; then
    SYS_GCC_VER=$(gcc --version | cut -d' ' -f3)
    if [[ ${SYS_GCC_VER/.*/} -ge $GCC_MAJOR_VER ]]; then
        export CC=gcc
        export CXX=g++
        unset AR
        unset NM
    else
        export INFRA_ROOT=/opt/infra.1
        export CC=$INFRA_ROOT/bin/gcc-$GCC_MAJOR_VER
        export CXX=$INFRA_ROOT/bin/g++-$GCC_MAJOR_VER
        export AR=$INFRA_ROOT/bin/gcc-ar-$GCC_MAJOR_VER
        export NM=$INFRA_ROOT/bin/gcc-nm-$GCC_MAJOR_VER
        export LDFLAGS="${LDFLAGS-} -Wl,-rpath=$INFRA_ROOT/lib64"
    fi
elif [[ $compiler == clang ]]; then
    SYS_CLANG_VER=$(clang++ --version | head -1 | cut -d' ' -f3)
    if [[ ${SYS_CLANG_VER/.*/} -ge $CLANG_MAJOR_VER ]]; then
        export CC=clang
        export CXX=clang++
        unset AR
        unset NM
    else
        export INFRA_ROOT=/opt/infra.1
        export CC=$INFRA_ROOT/bin/clang-$CLANG_MAJOR_VER
        export CXX=$INFRA_ROOT/bin/clang++-$CLANG_MAJOR_VER
        export AR=$INFRA_ROOT/bin/llvm-ar-$CLANG_MAJOR_VER
        export NM=$INFRA_ROOT/bin/llvm-nm-$CLANG_MAJOR_VER
        export LDFLAGS="${LDFLAGS-} -Wl,-rpath=$INFRA_ROOT/lib64"
    fi
fi
