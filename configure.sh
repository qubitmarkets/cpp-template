#!/bin/bash

set -euxo pipefail

cd $(dirname ${BASH_SOURCE})
ROOT_DIR=$PWD

./3rdparty/build.sh

cmake --preset $profile.$compiler

#CMAKE_DIR=$ROOT_DIR/target/debug.clang
#cmake -DCMAKE_INSTALL_PREFIX=$CMAKE_DIR -DCMAKE_C_COMPILER=/usr/bin/clang-18 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 -DCMAKE_BUILD_TYPE=Debug -S$ROOT_DIR -B$CMAKE_DIR -G Ninja
#cmake --build $CMAKE_DIR
