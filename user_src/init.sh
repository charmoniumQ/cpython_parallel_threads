#!/bin/sh

set -x -e

echo 8 > /proc/sys/kernel/printk
lines=$(dmesg | wc -l)
cat /proc/cpuinfo | grep wp
gcc test.c -o test
./test
dmesg | tail -n +${lines}
shutdown -h now
