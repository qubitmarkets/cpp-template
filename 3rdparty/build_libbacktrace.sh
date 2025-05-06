#!/bin/bash

set -euxo pipefail

cd $(dirname ${BASH_SOURCE})
source ../etc/utils.sh

install_dir=$PWD

if [[ ! -d libbacktrace ]]; then
    run git clone https://github.com/ianlancetaylor/libbacktrace.git
fi
run cd libbacktrace
run mkdir -p build
run cd build
run ../configure
run make
run make install prefix= DESTDIR=$install_dir

echo -e "\n\nlibbacktrace installed to $install_dir/lib"
