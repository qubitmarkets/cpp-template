# cpp-template

This repo creates a framework for building C++ projects.  It builds compilers, build tools like cmake, and some 3rdparty libraries.  It installs useful development scripts ()`./etc`) to `/usr/bin`

## Usage

This will copy the files to the desination dir.  It will not overwite any modified files, so it's safe to use for updating.

`./install.sh <destination_dir>`

- Change to the destination dir
- Run the `./prereqs.sh` script to install git hooks and build all the tooling
- Run the `./etc/install.sh` script to install useful dev scripts to `/usr/bin`
- Run the `./3rdparty/build.sh` script to update the 3rdparty library builds

## Adding a new compiler version

- Update the default version in `build.sh`, and (`build_gcc.sh` / `build_clang.sh`).
- Add the compiler & preset variants to `CMakePresets.json`
- Test the build & commit
