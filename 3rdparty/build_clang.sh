#!/bin/bash

CLANG_VER=${CLANG_VER-19}
CLANG_MAJOR_VER=${CLANG_VER%%.*}

# Install clang
if [[ -e /usr/bin/clang++-$CLANG_VER ]]; then exit 0; fi

if [[ -e /usr/bin/apt ]]; then
    sudo apt-get install -y libstdc++-12-dev
else
    sudo dnf install -y libstdc++-12-devel
fi
wget -q0- https://apt.llvm.org/llvm.sh | sudo bash -s $CLANG_VER
