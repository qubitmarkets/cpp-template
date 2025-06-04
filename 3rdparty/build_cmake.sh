#!/bin/bash

VER=4.0.2
DESTDIR=/opt/infra.1

cd $(dirname $0)

wget https://github.com/Kitware/CMake/releases/download/v${VER}/cmake-${VER}.tar.gz -O /tmp/cmake-${VER}.tar.gz
tar -xzf /tmp/cmake-${VER}.tar.gz
rm /tmp/cmake-${VER}.tar.gz
cd cmake-${VER}
./bootstrap --prefix=$DESTDIR --parallel=$(nproc)
make -j$(nproc)
sudo make install
sudo ln -nfs $DESTDIR/bin/cmake /usr/bin/cmake
