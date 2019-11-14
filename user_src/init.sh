#!/bin/sh

set -x -e

lines=$(dmesg | wc -l)

echo 8 > /proc/sys/kernel/printk
mod_dir=/lib/modules/$(uname -r)
module="/execves.ko"
mkdir -p "${mod_dir}"
echo "${module}" >> "${mod_dir}/modules.order"
touch "${mod_dir}/modules.builtin"
depmod -a

insmod "${module}"
gcc test2.c -o test2
gcc test.c -o test
# gdb ./test -ex r
./test
rmmod $(basename "${module}")

dmesg | tail -n +${lines}
shutdown -h now
