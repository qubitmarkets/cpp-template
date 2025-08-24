#!/bin/bash

find qcore/ -type f -name "*.cc" -o -name "*.h" | xgrep NOCOMMIT && exit 1
