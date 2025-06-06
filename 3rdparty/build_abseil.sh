#!/bin/bash

set -eu

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh

echo "-------------------------"
echo "Building abseil"
echo "-------------------------"

ver=lts_2025_05_12

run rm -rf catch2
if [[ -d abseil-cpp ]]; then
    # run git -C abseil-cpp pull origin $ver
    true
else
    run git clone https://github.com/abseil/abseil-cpp.git abseil-cpp -b $ver
fi
run cd abseil-cpp
ln -nfs ../CMakePresets.json CMakePresets.json
mkdir -p $build_dir

run cmake -S . -B $build_dir \
    -G Ninja --preset $profile.$compiler -DCMAKE_INSTALL_PREFIX=$install_dir -DCMAKE_INSTALL_LIBDIR=lib \
    -DABSL_BUILD_TESTING=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=23

ninja -v -C $build_dir
ninja -v -C $build_dir install
