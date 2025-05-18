#!/bin/bash

set -eux
source ./toolchain.sh

cd $(dirname ${BASH_SOURCE})
if [[ ! -e f-stack ]]; then
    git clone git@github.com:qubitmarkets/f-stack.git
fi
cd f-stack

export FF_PATH=$PWD
export DPDK_BUILD_DIR=$FF_PATH/dpdk/build

if ! which pyenv >/dev/null; then
    curl https://pyenv.run | bash
    export PYENV_ROOT="$HOME/.pyenv"
    [[ -d $PYENV_ROOT/bin ]] && export PATH="$PYENV_ROOT/bin:$PATH"
    eval "$(pyenv init --path)"
fi
export PYENV_ROOT="${PYENV_ROOT-$HOME/.pyenv}"
if [[ ! -e $PYENV_ROOT/versions/venv/bin/activate ]]; then
    pyenv virtualenv venv
fi
source $PYENV_ROOT/versions/venv/bin/activate

if [[ -e /usr/bin/apt ]]; then
    sudo apt-get install -y linux-headers-generic build-essential pkg-config libnuma-dev libssl-dev openssl meson ninja-build libpcap-dev python3-pip
else
    sudo yum install -y kernel-devel-$(uname -r) kernel-headers meson ninja-build numactl-devel openssl-devel libpcap-devel python libfdt-devel
fi

pip3 install pyelftools --upgrade

cd dpdk
export DESTDIR=/usr/local
mflags=""
if [[ -d build ]]; then
    mflags="--reconfigure"
fi
meson -Denable_kmods=true -Ddisable_libs=flow_classify -Ddisable_drivers=net/ice $mflags build
ninja -C build
sudo ninja -C build install

cd $FF_PATH/..

# Build f-stack
cd $FF_PATH/lib
make CFLAGS="-Wno-dangling-pointer"
sudo make install

echo
echo "Done"
echo
