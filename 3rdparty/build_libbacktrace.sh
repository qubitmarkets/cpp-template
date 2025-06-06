#!/bin/bash

set -euo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh

echo "-------------------------"
echo "Building libbacktrace"
echo "-------------------------"

if [[ ! -d libbacktrace ]]; then
    run git clone https://github.com/ianlancetaylor/libbacktrace.git
fi
run cd libbacktrace
src_dir=$PWD

run mkdir -p $build_dir
run cd $build_dir
run $src_dir/configure
run make
run make install prefix= DESTDIR=$install_dir

mkdir -p $install_dir/lib/cmake
cp $src_dir/../libbacktrace.cmake $install_dir/lib/cmake/

echo -e "\n\nlibbacktrace installed to $install_dir/lib"
