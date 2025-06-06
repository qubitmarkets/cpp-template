#!/bin/bash

cd $(dirname $BASH_SOURCE)
cd ..

COLOR_RED="\x1b[1;31m"
COLOR_NONE="\x1b[m"

missing=()
fix=0
git_staged_only=0
for arg in $@; do
  if [[ "$arg" == "-f" ]]; then
    fix=1
  elif [[ "$arg" == "-c" ]]; then
    git_staged_only=1
  fi
done

if [[ $git_staged_only == 1 ]]; then
  files=($(git status --porcelain | /bin/grep '^[AM]' | cut -c4- | grep '\.h$|\.cc$'))
else
  files=($(find qbuild/ qcore/ -name '*.h' -o -name '*.cc'))
fi

if [[ "${#files[@]}" == 0 ]]; then
  exit
fi

for f in "${files[@]}"; do
  if ! head -1 $f | grep -q -E '^// Copyright \(c\) 2025 Qubit Markets Pte\. Ltd\.$'; then
    missing+=("$f")
  fi
done
  
if [[ ${#missing[@]} -gt 0 ]]; then
  if [[ $fix == 0 ]]; then
    echo -e "${COLOR_RED}[ERROR]${COLOR_NONE} Copyright header missing from :"
    for f in "${missing[@]}"; do
      echo "  $f"
    done
    echo "Run $0 -f to fix"
    exit 1
  else
    for f in "${missing[@]}"; do
      echo "Adding copyright header to $f"
      sed -i '1s,^,// Copyright (c) 2025 Qubit Markets Pte. Ltd.\n,' "$f"
    done
    # Rerun check
    exec "$0"
  fi
fi
  
