#!/bin/bash

set -eu
cd $(dirname ${BASH_SOURCE})

source ./utils.sh

check_sudo

echo "Install dev tools"
sudo cp -f ./tmux-signal /usr/bin
sudo cp -f ./signal-all-bash /usr/bin
sudo cp -f ./find-root /usr/bin
sudo cp -f ./ninja-root-build /usr/bin
sudo cp -f ./build-run /usr/bin
sudo cp -f ./colorpath /usr/bin

if ! grep -q "bind.*tmux-signal" ~/.tmux.conf; then
    echo 'bind -T root C-t run-shell -b "tmux-signal #{pane_pid}' >>~/.tmux.conf
    echo "Reload tmux"
fi
if ! grep -q "source .*build-run.sh" ~/.bashrc; then
  echo "(optional) Add to bashrc:"
  echo "  source ~/.build-run.sh"
fi
(
echo "alias b='build-run build'"
echo "alias r='build-run run'"
echo "alias t='build-run test'"
echo "alias n='ninja-build'"
) > ~/.build-run.sh
