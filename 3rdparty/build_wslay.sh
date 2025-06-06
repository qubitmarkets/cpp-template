#!/bin/bash

cd $(dirname $BASHSOURCE[0])
source ./toolchain.sh

echo "-------------------------"
echo "Building wslay"
echo "-------------------------"

if [[ ! -d wslay ]]; then
    git clone https://github.com/tatsuhiro-t/wslay.git
fi
cd wslay
src_dir=$PWD

set -e

mkdir -p $build_dir
autoreconf -i
automake
autoconf
cd $build_dir
$src_dir/configure --prefix=$install_dir
make install

if [[ $? -ne 0 ]]; then
    echo "Failed to build wslay"
    exit 1
fi
