#!/bin/bash

set -euo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh
source ./toolchain.sh
pkg=openssl
version=3.5.2

echo "-------------------------"
echo "Building $pkg $version"
echo "-------------------------"

root=$PWD
tarfile=$pkg-$version.tar.gz
sudo dnf install -y make gcc perl-core pcre-devel wget zlib-devel zlib-static
if [[ ! -e $tarfile && ! -e $pkg-$version ]]; then
  wget https://github.com/openssl/openssl/releases/download/$pkg-$version/$tarfile -O $tarfile
  wget https://github.com/openssl/openssl/releases/download/$pkg-$version/$tarfile.sha256 -O $tarfile.sha256
fi
sha256sum --check $tarfile.sha256
tar -xzf $tarfile
cd $pkg-$version

# Build & install
#mkdir -p build
#cd build
./config no-shared --prefix=$install_dir --openssldir=$install_dir/ssl --with-zlib-lib=/usr/lib64/libz.a
make -j$(nproc)
make install_sw

cd $root
rm ./$tarfile
rm ./$tarfile.sha256
#rm -rf ./$pkg-$version
echo "Done"
