#!/bin/env python3

import sys, os, glob
# genie installs each gcc major in its own prefix: <root>/gcc-<major>/share/gcc-<version>
root = os.environ.get('INFRA_ROOT', '/opt/infra.1')
gcc_dirs = glob.glob(f'{root}/gcc-*/share/gcc-*') + glob.glob(f'{root}/share/gcc-*')
# Choose the latest version
gcc_dirs.sort(key=lambda d: [int(x) for x in os.path.basename(d).replace('gcc-', '').split('.') if x.isdigit()])
gcc_dir = gcc_dirs[-1]
gcc_ver = os.path.basename(gcc_dir).replace('gcc-','')
sys.path.insert(0, f"{gcc_dir}/python")
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers (None)
print(f"GCC {gcc_ver} libstdcxx Pretty Printers loaded")
