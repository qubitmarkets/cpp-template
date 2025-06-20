#!/bin/bash

CLANG_VER=${CLANG_VER-$1}
CLANG_MAJOR_VER=${CLANG_VER%%.*}
#export DESTDIR=${INFRA_ROOT-/opt/infra.1}

cd $(dirname $0)

set -eu

if [[ -z $CLANG_VER ]]; then
    echo "Usage: $0 <clang-major-version>"
    echo "Example: $0 20"
    exit 1
fi

# Install clang
#if [[ -e /usr/bin/clang++-$CLANG_VER ]]; then exit 0; fi

if [[ -e /usr/bin/apt ]]; then
    sudo apt-get install -y libstdc++-12-dev
    wget -q0- https://apt.llvm.org/llvm.sh | sudo bash -s $CLANG_VER
else
    # Build from source

    if [[ ! -d llvm-project ]]; then
        git clone https://github.com/llvm/llvm-project.git
    fi
    cd llvm-project
    git checkout release/${CLANG_MAJOR_VER}.x

    BUILD_DIR=build-${CLANG_MAJOR_VER}
    mkdir -p $BUILD_DIR && cd $BUILD_DIR
    cmake -G Ninja ../llvm \
        -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld;lldb;polly;mlir" \
        -DCLANG_DEFAULT_PIE_ON_LINUX=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$INFRA_ROOT

    ninja
    sudo ninja install
    sudo ln -nsf clang-${CLANG_MAJOR_VER} $INFRA_ROOT/bin/clang++-${CLANG_MAJOR_VER}
    if [[ ! -e /usr/bin/clang-$CLANG_MAJOR_VER ]]; then
        sudo ln -nfs $INFRA_ROOT/bin/clang-${CLANG_MAJOR_VER} /usr/bin/clang-${CLANG_MAJOR_VER}
        sudo ln -nfs $INFRA_ROOT/bin/clang++-${CLANG_MAJOR_VER} /usr/bin/clang++-${CLANG_MAJOR_VER}
    fi
    cmake --build $PWD --target clangd
    cmake --build $PWD --target check-clangd

    touch /tmp/empty.c
    $INFRA_ROOT/bin/clang-$CLANG_MAJOR_VER /tmp/empty.c -fsyntax-only

fi
