# cpp-template

This repo creates a framework for building C++ projects.  It uses compilers, cmake and 3rdparty libraries from `/opt/infra.1`, built by [genie](../genie) (`~/qbm/genie`).  It installs useful development scripts ()`./etc`) to `/usr/bin`

## Usage

This will copy the files to the desination dir.  It will not overwite any modified files, so it's safe to use for updating.

`./install.sh <destination_dir>`

- Change to the destination dir
- Run the `./prereqs.sh` script to install git hooks and build all the tooling
- Run the `./etc/install.sh` script to install useful dev scripts to `/usr/bin`
- To update the compilers and 3rdparty libraries: `~/qbm/genie/build.sh` builds them in a container, then `gpm install --repo /build/$USER/genie/packages --all` installs them to `/opt/infra.1` (`prereqs.sh` does both; set `GENIE_ROOT` if genie is elsewhere)

Libraries are read from `/opt/infra.1/<profile>.<compiler>`. To use libraries installed elsewhere (`gpm install --root ~/infra ...`), set `THIRD_PARTY_ROOT` (e.g. `export THIRD_PARTY_ROOT=~/infra`) for cmake configure; `qbuild.cmake` swaps it in for `/opt/infra.1`.

## Adding a new compiler version

Compilers and libraries are built in genie: follow "Adding a new compiler version" in genie's `README.md` first. Then, in this repo:

- Add the compiler, sanitize and profile presets to `CMakePresets.json` (`THIRD_PARTY_DIR` is `/opt/infra.1/<profile>.<compiler>`), and point `clang-tidy` at the new clang.
- Update the default `compiler` in `build-run` and the compiler loop in `tests.sh`.
- Test the build & commit
