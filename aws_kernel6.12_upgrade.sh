#!/bin/bash

cur_minor=$(uname -r | cut -d. -f2)
if [[ $cur_minor -lt 12 ]]; then
  sudo dnf install -y kernel6.12
  version=$(rpm -q --qf '%{version}-%{release}.%{arch}\n' kernel6.12 | sort -V | tail -1) 
  sudo grubby --set-default "/boot/vmlinuz-$version" 
  read -p "Press any key to reboot.  Run $0 after reboot"
  sudo reboot 
else
  sudo dnf install -y kernel6.12-modules-extra-$(uname -r) kernel-headers-$(uname -r) kernel-devel-$(uname -r) 
  sudo dnf -y swap perf perf6.12 
fi
