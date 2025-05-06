#!/bin/bash

set -eux

# Installs gcc to /opt/infra.1/bin

# Creates g++-14
GCC_MAJOR_VER=${GCC_MAJOR_VER-14}
DESTDIR=${DESTDIR-/opt/infra.1}

cd $(dirname ${BASH_SOURCE})

if [[ -e /usr/bin/apt ]]; then
    sudo apt install -y build-essential gcc flex m4 bison tcl texinfo autoconf automake
elif [[ -e /usr/bin/dnf ]]; then
    sudo dnf install -y gcc flex m4 bison tcl texinfo autoconf automake
fi

if [[ ! -d gcc ]]; then
    #git clone --depth=1 git://gcc.gnu.org/git/gcc.git
    git clone --depth=1 -b releases/gcc-${GCC_MAJOR_VER} git@github.com:gcc-mirror/gcc.git
fi
cd gcc
git checkout releases/gcc-$GCC_MAJOR_VER
./contrib/download_prerequisites
mkdir -p build
cd build
../configure --prefix=$DESTDIR \
    --enable-languages=c,c++ \
    --disable-multilib \
    --program-suffix "-$GCC_MAJOR_VER"
make
sudo make install
