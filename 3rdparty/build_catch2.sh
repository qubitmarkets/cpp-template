#!/bin/bash

set -euxo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh

install_dir=$PWD

run rm -rf catch2
run git clone --depth 10 https://github.com/catchorg/Catch2.git catch2
run cd catch2
run cmake -S . -B build -G Ninja -DCMAKE_INSTALL_LIBDIR=lib
run cmake --build build
run cmake --install build --prefix $install_dir
