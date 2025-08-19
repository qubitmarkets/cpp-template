#!/bin/bash

echo "Tidying code"

export PATH=$PATH:/opt/infra.1/bin

set -eux

dirs=""
for d in qbuild qcore; do if [[ -e $d ]]; then
  dirs="$dirs $d"
fi; done
find $dirs -type f -name "*.cc" -o -name "*.h" | xargs clang-format -i
shfmt -ci -i 4 -w ./build-run
