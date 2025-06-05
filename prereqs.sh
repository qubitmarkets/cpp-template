#!/bin/bash

set -eu
source ./etc/utils.sh

check_sudo

cp etc/git-hooks/pre-commit .git/hooks/

if [[ -e /usr/bin/dnf ]]; then
    run sudo dnf install -y ninja-build openssl-devel libtool
elif [[ -e /usr/bin/apt ]]; then
    run sudo apt install -y ninja-build libssl-dev
fi

# Install clang & cmake from source
run 3rdparty/build.sh
run etc/install.sh
