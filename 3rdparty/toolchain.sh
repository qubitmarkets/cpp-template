#/bin/bash

export INFRA_ROOT=/opt/infra.1
export install_root=${install_root-$(readlink -f $(dirname ${BASH_SOURCE}))}
export compiler=${compiler-gcc15}
export profile=release${dot_profile_extras-}
export build_dir=builds/$profile.$compiler
export install_dir=$install_root/install/$profile.$compiler

cat ../CMakePresets.json | sed 's,-include ${sourceDir}/qbuild/qbuild.h,,' >CMakePresets.json

unset CFLAGS
unset LDFLAGS
if [[ $profile =~ "sanitize" ]]; then
    export CFLAGS="-fsanitize=address -fno-omit-frame-pointer"
    export LDFLAGS="-fsanitize=address"
fi

if [[ ${compiler:0:3} == gcc ]]; then
    GCC_MAJOR_VER=${compiler:3:2}
    if [[ $GCC_MAJOR_VER == "" ]]; then
        export CC=gcc
        export CXX=g++
        unset AR
        unset NM
    else
        export CC=$INFRA_ROOT/bin/gcc-$GCC_MAJOR_VER
        export CXX=$INFRA_ROOT/bin/g++-$GCC_MAJOR_VER
        export AR=$INFRA_ROOT/bin/gcc-ar-$GCC_MAJOR_VER
        export NM=$INFRA_ROOT/bin/gcc-nm-$GCC_MAJOR_VER
        export CFLAGS="${CFLAGS-}"
        export LDFLAGS="-Wl,-rpath=$INFRA_ROOT/lib64 -L$INFRA_ROOT/lib64 ${LDFLAGS-}"
    fi
elif [[ ${compiler:0:5} == clang ]]; then
    CLANG_MAJOR_VER=${CLANG_MAJOR_VER:5:2}
    if [[ $CLANG_MAJOR_VER == "" ]]; then
        export CC=clang
        export CXX=clang++
        unset AR
        unset NM
    else
        export CC=$INFRA_ROOT/bin/clang-$CLANG_MAJOR_VER
        export CXX=$INFRA_ROOT/bin/clang++-$CLANG_MAJOR_VER
        export AR=$INFRA_ROOT/bin/llvm-ar-$CLANG_MAJOR_VER
        export NM=$INFRA_ROOT/bin/llvm-nm-$CLANG_MAJOR_VER
        export CFLAGS="${CFLAGS-} -fuse-ld=lld"
        export LDFLAGS="-Wl,-rpath=$INFRA_ROOT/lib64 -L$INFRA_ROOT/lib64 ${LDFLAGS-}"
    fi
fi
