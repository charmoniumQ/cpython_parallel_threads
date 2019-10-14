#!/bin/sh

set -x -e

echo 8 > /proc/sys/kernel/printk
insmod ./execves.ko
gcc test.c -o test
./test
rmmod execves
dmesg
shutdown -h now
