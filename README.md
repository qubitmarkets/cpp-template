# cpp-template

This repo creates a framework for building C++ projects.  It uses compilers, cmake and 3rdparty libraries from `/opt/infra.1`, built by [genie-packages](../genie-packages) (`~/qbm/genie-packages`).  It installs useful development scripts ()`./etc`) to `/usr/bin`

## Usage

This will copy the files to the desination dir.  It will not overwite any modified files, so it's safe to use for updating.

`./install.sh <destination_dir>`

- Change to the destination dir
- Run the `./prereqs.sh` script to install git hooks and build all the tooling
- Run the `./etc/install.sh` script to install useful dev scripts to `/usr/bin`
- To update the compilers and 3rdparty libraries: `~/qbm/genie-packages/build.sh` builds them in a container, then `gpm install --repo /build/$USER/genie/packages --all` installs them to `/opt/infra.1` (`prereqs.sh` does both; set `GENIE_ROOT` if genie is elsewhere)

The CMake presets come from genie: `CMakePresets.json` only includes `/opt/infra.1/share/genie/CMakePresets.json` (gpm package `genie-presets`), which has `debug|release[.sanitize].<compiler>` for every compiler genie builds, and `clang-tidy`. They set the compilers, `THIRD_PARTY_DIR` (`/opt/infra.1/<profile>.<compiler>`), a gcc's rpath and the sanitize flags; this repo's own flags (`-include qbuild.h`, ...) are in `qbuild.cmake`.

`etc/infra.sh` holds the rest: `INFRA_ROOT`, the default compiler for `build-run` and the compilers `tests.sh` tests. `etc/` is shared with the repos made from this one, so a change there applies to all of them.

To use libraries installed elsewhere (`gpm install --root ~/infra ...`), set `THIRD_PARTY_ROOT` (e.g. `export THIRD_PARTY_ROOT=~/infra`) for cmake configure; `qbuild.cmake` swaps it in for `/opt/infra.1`.

## Adding a new compiler version

Follow "Adding a new compiler version" in genie's `README.md`, then `gpm install --repo /build/$USER/genie/packages --all`. Its presets appear in every repo through `genie-presets`. To make it the default or test it, change `DEFAULT_COMPILER` or `TEST_COMPILERS` in `etc/infra.sh`, then run `./tests.sh`.

## Moving the root

Build genie with `INFRA_ROOT=<root> ./build.sh` and install with `gpm install --root <root> ...`. Then change `INFRA_ROOT` in `etc/infra.sh` and the `include` path in `CMakePresets.json` (in each repo made from this one).
