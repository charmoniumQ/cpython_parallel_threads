#!/bin/sh

set -x -e

insmod /execves.ko
rmmod execves
dmesg
