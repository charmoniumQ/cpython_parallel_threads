#!/bin/sh

set -x -e

echo 8 > /proc/sys/kernel/printk
insmod /execves.ko
rmmod execves
dmesg
shutdown
