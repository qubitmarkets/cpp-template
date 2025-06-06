#!/bin/bash

todo=()

# Disable CPU frequency scaling
if grep -q 'scaling_governor' /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; then
    echo "CPU frequency scaling is disabled"
else
    echo "CPU frequency scaling is enabled"
    echo "Disabled CPU frequency scaling"
    todo+=("/etc/default/grub: Add cpufreq=performance")
fi
# Disable CPU C-states
if grep -q 'intel_idle.max_cstate' /proc/cmdline; then
    echo "CPU C-states are disabled"
else
    echo "CPU C-states are enabled"
    if ! grep -q 'intel_idle.max_cstate=0' /etc/default/grub; then
        todo+=("/etc/default/grub: Add intel_idle.max_cstate=0")
    fi
    echo "Disabled CPU C-states"
fi
# Disable CPU Turbo Boost
if grep -q 'intel_pstate=disable' /proc/cmdline; then
    echo "CPU Turbo Boost is disabled"
else
    echo "CPU Turbo Boost is enabled"
    todo+=("/etc/default/grub: Add intel_pstate=disable")
fi
# Disable CPU Hyper-Threading
if grep -q 'nosmt' /proc/cmdline; then
    echo "CPU Hyper-Threading is disabled"
else
    echo "CPU Hyper-Threading is enabled"
    todo+=("/etc/default/grub: Add nosmt")
fi
# Set poll=idle
if grep -q 'poll=idle' /proc/cmdline; then
    echo "poll=idle is set"
else
    echo "poll=idle is not set"
    todo+=("/etc/default/grub: Add poll=idle")
fi

# Disable Transparent Huge Pages
if grep '[never]' /sys/kernel/mm/transparent_hugepage/enabled; then
    echo "Transparent Huge Pages is disabled"
else
    echo "Transparent Huge Pages is enabled"
    todo+=("/etc/default/grub: Add transparent_hugepage=never")
fi

# Allocate huge pages at boot
total_mem_kb=$(grep MemTotal /proc/meminfo | awk '{print $2}')
num_2mb_hugepages=$(cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages)
max_num_hugepages=$((total_mem_kb / 2048))
ideal_num_hugepages=$((total_mem_kb / 2048 / 2))

echo "Num 2Mb Hugepages : $(cat /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages)"
if grep -q 'hugepagesz=2M' /etc/default/grub; then
    echo "2M Huge pages are allocated"
else
    echo "2M Huge pages are not allocated"
    todo+=("/etc/default/grub: Add hugepagesz=2M hugepages=$ideal_num_hugepages")
fi

echo "Num 1G Hugepages : $(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages)"
if grep -q 'hugepagesz=1G' /etc/default/grub; then
    echo "1G Huge pages are allocated"
else
    echo "1G Huge pages are not allocated"
    todo+=("/etc/default/grub: Add hugepagesz=2M hugepages=$ideal_num_hugepages")
fi

max_open_fds=$(ulimit -n)
if [[ $max_open_fds -lt 500000 ]]; then
  echo "Max file descriptor limit is too low ($max_open_fds)"
  todo+="/etc/security/limits.conf: Add '*   soft nofile  524288'"
  todo+="/etc/security/limits.conf: Add '*   hard nofile 1048576'"
fi

# Report TODO items
for t in "${todo[@]}"; do
    echo "+ $t"
done
