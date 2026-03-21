#!/bin/bash

set -eu
source ./etc/utils.sh

check_sudo

echo "Installing git hooks"
cp etc/git-hooks/pre-commit .git/hooks/

echo "Installing dev packages"
if [[ -e /usr/bin/dnf ]]; then
    run sudo dnf install -y ninja-build openssl-devel libtool sudo zlib lz4 snappy brotli zstd ccache
    run sudo dnf install -y zlib-devel lz4-devel snappy-devel libzstd-devel brotli-devel
elif [[ -e /usr/bin/apt ]]; then
sudo apt-get update && sudo apt-get install -y \
  ninja-build  libssl-dev  libtool  sudo  zlib1g  liblz4-1  libsnappy1v5  libbrotli1 libzstd1 ccache wget \
  zlib1g-dev liblz4-dev  libsnappy-dev   libzstd-dev   libbrotli-dev
fi

echo "Install useful dev scripts"
run etc/install.sh

echo "Install clang & cmake from source, 3rdparty libs"
run 3rdparty/build.sh
