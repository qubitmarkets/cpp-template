#!/bin/bash

CLANG_VER=${CLANG_VER-$1}
CLANG_MAJOR_VER=${CLANG_VER%%.*}

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
# Currently unsupported
fi
