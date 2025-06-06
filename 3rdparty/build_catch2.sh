#!/bin/bash

set -euo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh

echo "-------------------------"
echo "Building catch2"
echo "-------------------------"

if [[ ! -e catch2 ]]; then
    run git clone --depth 10 https://github.com/catchorg/Catch2.git catch2
fi
run cd catch2
ln -nfs ../CMakePresets.json CMakePresets.json
run cmake -S . -B $build_dir \
    -G Ninja --preset $profile.$compiler -DCMAKE_INSTALL_PREFIX=$install_dir -DCMAKE_INSTALL_LIBDIR=lib

run ninja -v -C $build_dir
run ninja -v -C $build_dir install
