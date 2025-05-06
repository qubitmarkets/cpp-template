#!/bin/bash

cd $(dirname ${BASH_SOURCE})

./build-run test
if [[ -e ./build/utils.t ]]; then
    echo "Run external tests"
    ./build/utils.t '[ext]'
fi
