#!/bin/sh

set -x -e

lines=$(dmesg | wc -l)
echo 8 > /proc/sys/kernel/printk
insmod ./execves.ko
gcc test.c -o test
./test
rmmod execves
dmesg | tail -n +${lines}
shutdown -h now
