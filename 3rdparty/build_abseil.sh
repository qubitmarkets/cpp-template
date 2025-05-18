#!/bin/bash

set -euxo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh

install_dir=$PWD

run rm -rf catch2
if [[ -d abseil-cpp ]]; then
    run git -C abseil-cpp pull origin lts_2025_01_27
else
    run git clone https://github.com/abseil/abseil-cpp.git abseil-cpp -b lts_2025_01_27
fi
run cd abseil-cpp
run cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX=$install_dir -DCMAKE_INSTALL_LIBDIR=lib -DABSL_BUILD_TESTING=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=23
run cmake --build build --target all
run ctest
run cmake --install build --prefix $install_dir
