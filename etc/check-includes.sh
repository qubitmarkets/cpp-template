#!/bin/bash

# Verify the include heirarchy

set -euo pipefail

include_hierarchy=(qbuild mem shm io nexus app feed)
src=qcore

n=${#include_hierarchy[@]}
n=$(($n - 1))
rv=0

for i in $(seq 0 $n); do
    a=${include_hierarchy[$i]}
    if [[ ! -d "$src/$a" ]]; then
        continue
    fi
    #echo "check-includes: $a/"
    for j in $(seq $(($i + 1)) $n); do
        b=${include_hierarchy[$j]}
        grep -r '# *include ["<]'"$b"'/' "$src/$a" && echo "No $b/ includes allowed in $src/$a" && rv=1
    done
done

exit $rv
