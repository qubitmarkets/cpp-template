#!/bin/bash

find qcore/ -type f -name "*.cc" -o -name "*.h" | xgrep NOCOMMIT
if [[ $? == 0 ]]; then
  exit 1
fi
