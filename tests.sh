#!/bin/bash

cd $(dirname ${BASH_SOURCE})

set -eu

for compiler in gcc15 clang21; do
  for profile_base in debug release; do
    for dot_extra in "" ".sanitize"; do
      export compiler
      export profile=${profile_base}${dot_extra}
      echo "Testing $profile.$compiler"
      ./build-run test --cfg
    done
  done
done

./build-run test
if [[ -e ./build/dev.t ]]; then
    echo "Run hidden tests"
    ./build/dev.t '[.]'
fi
if [[ -e ./build/io.t ]]; then
    echo "Run external tests"
    ./build/io.t '[ext]'
fi
if [[ -e ./build/callback.t && "$profile" == "release" ]]; then
    echo "Run perf tests"
    ./build/callback.t '[perf]'
fi
