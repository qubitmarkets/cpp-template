#!/bin/bash

set -eu
source ./etc/utils.sh

check_sudo

echo "Installing git hooks"
cp etc/git-hooks/pre-commit .git/hooks/

echo "Installing dev packages"
if [[ -e /usr/bin/dnf ]]; then
    run sudo dnf install -y ninja-build openssl-devel libtool sudo zlib lz4 snappy brotli zstd zlib-static
    run sudo dnf install -y zlib-devel lz4-devel snappy-devel libzstd-devel brotli-devel
elif [[ -e /usr/bin/apt ]]; then
    run sudo apt install -y ninja-build libssl-dev
fi

echo "Install useful dev scripts"
run etc/install.sh

echo "Build compilers, cmake and 3rdparty libs with genie (in a container), and install them to /opt/infra.1 with gpm"
GENIE_ROOT=${GENIE_ROOT-$HOME/qbm/genie}
GENIE_OUT=${GENIE_OUT-/build/$USER/genie}
GPM=${GPM-$HOME/qbm/gpm/target/x86_64-unknown-linux-musl/release/gpm}
if [[ ! -x $GENIE_ROOT/build.sh ]]; then
    echo "ERROR: genie not found at $GENIE_ROOT. Clone it there or set GENIE_ROOT"
    exit 1
fi
run $GENIE_ROOT/build.sh
if [[ ! -w /opt/infra.1 ]]; then
    echo "ERROR: /opt/infra.1 is not writable. Make it yours: sudo install -d -o $(id -un) -g $(id -gn) /opt/infra.1"
    exit 1
fi
run $GPM install --root /opt/infra.1 --repo $GENIE_OUT/packages --all
