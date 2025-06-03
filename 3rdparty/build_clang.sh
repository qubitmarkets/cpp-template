#!/bin/bash

CLANG_VER=${CLANG_VER-$1}
CLANG_MAJOR_VER=${CLANG_VER%%.*}
DESTDIR=${DESTDIR-/opt/infra.1}

set -eu

if [[ -z $CLANG_VER ]]; then
    echo "Usage: $0 <clang-major-version>"
    echo "Example: $0 20"
    exit 1
fi

# Install clang
if [[ -e /usr/bin/clang++-$CLANG_VER ]]; then exit 0; fi

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

mkdir -p build && cd build
cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld;lldb;polly;mlir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$DESTDIR

ninja
sudo ninja install
sudo ln -nsf clang-${CLANG_MAJOR_VER} $DESTDIR/bin/clang++-${CLANG_MAJOR_VER}

touch /tmp/empty.c
$DESTDIR/bin/clang-$CLANG_MAJOR_VER /tmp/empty.c -fsyntax-only

#export CC=$DESTDIR/bin/clang-$CLANG_MAJOR_VER
#export CXX=$DESTDIR/bin/clang++-$CLANG_MAJOR_VER
#ninja check-clang

fi
