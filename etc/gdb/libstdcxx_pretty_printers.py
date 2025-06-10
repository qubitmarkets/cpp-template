#!/bin/env python3

import sys, os, glob
gcc_dirs = glob.glob('/opt/infra.1/share/gcc-*')
# Choose the latest version
gcc_dirs.sort()
gcc_dir = gcc_dirs[-1]
gcc_ver = os.path.basename(gcc_dir).replace('gcc-','')
sys.path.insert(0, f"{gcc_dir}/python")
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers (None)
print(f"GCC {gcc_ver} libstdcxx Pretty Printers loaded")
