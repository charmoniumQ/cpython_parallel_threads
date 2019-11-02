#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include "execves_syscall.c"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Samuel Grayson");
MODULE_DESCRIPTION("Supports execve with (s)haring");
MODULE_VERSION("0.01");

SYSCALL_DEFINE1(not_implemented, char, x) { return -ENOSYS; }

static void set_syscall(unsigned int number, sys_call_ptr_t function) {
    struct page *sys_call_table_page = virt_to_page(&sys_call_table[number]);
    unsigned long cr0 = read_cr0();

    // Note: modify write_cr0 to not check the protected bit
    // Note: change page protection to not protect RO data
    // X86_CR0_WP
    write_cr0(cr0 & ~0x10000);
    set_pages_rw(sys_call_table_page, 1);
    sys_call_table[number] = function;
    set_pages_rw(sys_call_table_page, 0);
    write_cr0(cr0);
}

static int __init execves_module_init(void) {
    printk(KERN_DEBUG "execves: init\n");
    set_syscall(__NR_execves, __x64_sys_execves);
    return 0;
}


static void __exit execves_module_exit(void) {
    set_syscall(__NR_execves, __x64_sys_not_implemented);
    printk(KERN_DEBUG "execves: exit\n");
}

module_init(execves_module_init);
module_exit(execves_module_exit);

/*
 * Build Linux https://www.freecodecamp.org/news/building-and-installing-the-latest-linux-kernel-from-source-6d8df5345980/
 * Add a syscall https://www.kernel.org/doc/html/v4.10/process/adding-syscalls.html
 * Implement syscall https://medium.com/@ssreehari/implementing-a-system-call-in-linux-kernel-4-7-1-6f98250a8c38#.wjl4xo6hx
 * Intercepting syscall https://www.reddit.com/r/kernel/comments/a3k981/lkm_and_issues_intercepting_execveforkvforkclone/
 * Hooking sys_execve https://unix.stackexchange.com/questions/402712/hooking-sys-execve-on-linux-kernel-4-6-or-higher
 * Syscall lab https://linux-kernel-labs.github.io/master/lectures/syscalls.html

 * https://www.collabora.com/news-and-blog/blog/2017/01/16/setting-up-qemu-kvm-for-kernel-development/
 * - Use debootstrap to make a disk image, mount disk image on loopback
 * https://unix.stackexchange.com/questions/438101/qemu-emulate-own-system-to-test-kernel-modules
 * - Use debootstrap to make a disk image, but mount it with nbd
 * https://mgalgs.github.io/2012/03/23/how-to-build-a-custom-linux-kernel-for-qemu.html
 * - They roll their own initramfs

 * Compiling kernel modules
 * https://medium.com/@clem.boin/creating-a-minimal-kernel-development-setup-using-qemu-and-archlinux-987896954d84
 * http://www.lifl.fr/%7Elipari/courses/ase_lkp/ase_lkp.html
 * https://linux-kernel-labs.github.io/master/labs/kernel_modules.html
 * https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234?gi=600c712bc249
 * http://blog.stuffedcow.net/wp-content/uploads/2015/08/Makefile

Hacking the sys_call table:
Hard-code the address from System.map [3, 5] (but disable kaslr [4]), brute-force scan in kernel memory, use kall_sysm_on_each_symbol (??) [1]
Once you have the symbol table, do [2].

1. https://stackoverflow.com/questions/6610733/reading-kernel-memory-using-a-module/6614661
2. http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN976
3. https://stackoverflow.com/questions/1586481/sys-call-table-in-linux-kernel-2-6-18
4. https://unix.stackexchange.com/questions/424119/why-is-sys-call-table-predictable
5. https://web.iiit.ac.in/~arjun.nath/random_notes/modifying_sys_call.html

 * Debugging in GDB
 * https://nickdesaulniers.github.io/blog/2018/10/24/booting-a-custom-linux-kernel-in-qemu-and-debugging-it-with-gdb/
 * https://www.kernel.org/doc/html/v4.10/dev-tools/gdb-kernel-debugging.html
 * https://ownyourbits.com/2018/05/09/debugging-the-linux-kernel/
 */
