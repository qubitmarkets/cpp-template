#!/bin/bash

set -eu
cd $(dirname $0)
source ./toolchain.sh

echo "-------------------------"
echo "Building simdjson"
echo "-------------------------"

if [[ ! -e simdjson ]]; then
    git clone --depth=1 git@github.com:simdjson/simdjson.git
fi
cd simdjson
ln -nfs ../CMakePresets.json CMakePresets.json
rm -rf $build_dir || true
mkdir -p $build_dir

set -x
cmake -B $build_dir -S . \
    -G Ninja --preset $profile.$compiler -DCMAKE_INSTALL_PREFIX=$install_dir -DCMAKE_INSTALL_LIBDIR=lib \
    -DSIMDJSON_EXCEPTIONS=OFF -DSIMDJSON_BUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=OFF -DSIMDJSON_AVX512_ALLOWED=0 \
    -DSIMDJSON_ENABLE_THREADS=0 -DSIMDJSON_SKIPUTF8VALIDATION=1 -DSIMDJSON_DISABLE_DEPRECATED_API=1 \
    -DSIMDJSON_BUILD_STATIC_LIB=ON -DPOSITION_INDEPENDENT_CODE=OFF

ninja -v -C $build_dir
ninja -C $build_dir install
