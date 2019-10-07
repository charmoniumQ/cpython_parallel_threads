#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Samuel Grayson");
MODULE_DESCRIPTION("Supports execve with (s)haring");
MODULE_VERSION("0.01");

asmlinkage long sys_ni_syscall(void) { return -ENOSYS; }

extern void *sys_call_table[];

typedef struct {
    int unused;
} execves_attr_t;

// Find first unused syscall number in linux/arch/x86/entry/syscalls/syscall_64.tbl
#define __NR_execves 436

asmlinkage long sys_execves(const char *pathname, char *const argv[], char *const envp[], const execves_attr_t* attr) {
    printk(KERN_EMERG "Called execves!\n");
    return 0;
}

static int __init execves_module_init(void) {
    printk(KERN_EMERG "Init execves!\n");
    sys_call_table[__NR_execves] = sys_execves;
    return 0;
}


static void __exit execves_module_exit(void) {
    sys_call_table[__NR_execves] = sys_ni_syscall;
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

 * Debugging in GDB
 * https://nickdesaulniers.github.io/blog/2018/10/24/booting-a-custom-linux-kernel-in-qemu-and-debugging-it-with-gdb/
 * https://www.kernel.org/doc/html/v4.10/dev-tools/gdb-kernel-debugging.html
 */
