#!/bin/sh

set -x -e

lines=$(dmesg | wc -l)
echo 8 > /proc/sys/kernel/printk
gcc test.c -o test
./test
dmesg | tail -n +${lines}
shutdown -h now
