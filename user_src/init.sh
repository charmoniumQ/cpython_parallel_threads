#!/bin/sh

set -x -e

echo 8 > /proc/sys/kernel/printk
lines=$(dmesg | wc -l)
insmod ./execves.ko
gcc test.c -o test
./test
rmmod execves
dmesg | tail -n +${lines}
shutdown -h now
