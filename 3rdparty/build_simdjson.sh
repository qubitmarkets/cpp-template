#!/bin/bash

set -eu
source ./toolchain.sh

cd $(dirname $0)

DESTDIR=$(readlink -f .)

if [[ ! -e simdjson ]]; then
    git clone --depth=1 git@github.com:simdjson/simdjson.git
fi
cd simdjson
rm -rf build || true
mkdir -p build

set -x
cmake -B build . -DSIMDJSON_EXCEPTIONS=OFF -G Ninja \
    -DSIMDJSON_BUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=ON -DSIMDJSON_AVX512_ALLOWED=0 -DSIMDJSON_ENABLE_THREADS=0 -DSIMDJSON_SKIPUTF8VALIDATION=1 \
    -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_C_COMPILER=$CC \
    -DCMAKE_INSTALL_PREFIX=$DESTDIR -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_BUILD_TYPE=Release
ninja -v -C build
ninja -C build install
