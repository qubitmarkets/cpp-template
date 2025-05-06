#!/bin/bash

set -eu
source ./etc/utils.sh

check_sudo

if [[ -e /usr/bin/dnf ]]; then
    run sudo dnf install -y ninja-build openssl-devel
else
    run sudo apt install -y ninja-build libssl-dev
fi

# Install clang & cmake from source
run 3rdparty/build.sh
run etc/install.sh
