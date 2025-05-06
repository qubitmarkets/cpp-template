#!/bin/bash

echo "Tidying code"

set -eux

find src/ tests/ -type f -name "*.cc" -o -name "*.h" | xargs clang-format -i
shfmt -ci -i 4 -w ./build-run
