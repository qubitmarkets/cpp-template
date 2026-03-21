#!/bin/bash

set -euo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh
pkg=numactl
version=2.0.19

echo "-------------------------"
echo "Building $pkg $version"
echo "-------------------------"

root=$PWD
tarfile=$pkg-${version}.tar.gz
if [[ -e /usr/bin/dnf ]]; then
  sudo dnf install -y make gcc perl-core pcre-devel wget zlib-devel zlib-static
else
sudo apt-get update && sudo apt-get install -y \
  make \
  gcc \
  perl \
  libpcre3-dev \
  wget \
  zlib1g-dev \
  zlib1g
fi
if [[ ! -e $tarfile ]]; then
  wget https://github.com/numactl/numactl/archive/refs/tags/v$version.tar.gz -O $tarfile
fi
tar -xzf $tarfile
cd $pkg-$version

# Build & install
#mkdir -p build
./autogen.sh
./configure --prefix=$install_dir --disable-shared
make -j$(nproc)
make install

cd $root
#rm ./$tarfile
#rm ./$tarfile.sha256
#rm -rf ./$pkg-$version
echo "Done"
