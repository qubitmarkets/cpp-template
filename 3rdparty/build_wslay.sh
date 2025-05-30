#!/bin/bash

source ./toolchain.sh

cd $(dirname $BASHSOURCE[0])
install_dir=$PWD

if [[ ! -d wslay ]]; then
    git clone https://github.com/tatsuhiro-t/wslay.git
fi
cd wslay

set -e

autoreconf -i
automake
autoconf
./configure --prefix=$install_dir
make install

if [[ $? -ne 0 ]]; then
    echo "Failed to build wslay"
    exit 1
fi
