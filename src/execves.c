#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/binfmts.h>
#include "execves.h"

//#define RESOURCE_(acquire, use, free) { acquire_block; do { use_block; } while (0); free_block; }
#define acquire {
#define use do
#define release while (0);
#define end_resource }
#define cleanup break

// symbols from fs/exec.c
extern int prepare_bprm_creds(struct linux_binprm *bprm);
extern void free_bprm(struct linux_binprm *bprm);
extern void check_unsafe_exec(struct linux_binprm *bprm);
extern struct file *do_open_execat(int fd, struct filename *name, int flags);
extern int bprm_mm_init(struct linux_binprm *bprm);

SYSCALL_DEFINE4(execves,
		const char __user *, filename,
		const char __user * const __user *, argv,
		const char __user * const __user *, envp,
		const execves_attr_t __user *, attr
) {
	unsigned long ret = 0;

	execves_attr_t attr_;
	/* struct user_arg_ptr argv_ = { .ptr.native = argv }; */
	/* struct user_arg_ptr envp_ = { .ptr.native = envp }; */

	printk(KERN_DEBUG "In syscall\n");

	use {
		acquire struct filename* filename_ = getname(filename);
		{
			if (unlikely(IS_ERR(filename_))) {
				ret = PTR_ERR(filename_);
				cleanup;
			}
		} use {
			printk(KERN_DEBUG "execves: called %s\n", filename_->name);

			while (unlikely((ret = copy_from_user(&attr_, attr, sizeof(attr_))) > 0));
			if (unlikely(ret < 0))
				cleanup;

			if ((current->flags & PF_NPROC_EXCEEDED) &&
				atomic_read(&current_user()->processes) > rlimit(RLIMIT_NPROC)) {
				ret = -EAGAIN;
				cleanup;
			}

			acquire struct linux_binprm *bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
			{
				if (unlikely(!bprm)) {
					ret = -ENOMEM;
					cleanup;
				}
			} use {
				ret = prepare_bprm_creds(bprm);
				if (ret)
					cleanup;

				check_unsafe_exec(bprm);

				acquire current->in_execve = 1;
				use {
					struct file* file = do_open_execat(AT_FDCWD, filename_, 0);
					ret = PTR_ERR(file);
					if (IS_ERR(file))
						cleanup;

					sched_exec();

					bprm->file = file;
					if (!filename_) {
						bprm->filename = "none";
					} else {
						bprm->filename = filename_->name;
					}
					bprm->interp = bprm->filename;

					ret = bprm_mm_init(bprm);
					if (ret)
						cleanup;

				} release {
					/* current->fs->in_exec = 0; */
					current->in_execve = 0;
				} end_resource;
			} release {
				free_bprm(bprm);
			} end_resource;
		} release {
			if (filename_)
				putname(filename_);
		} end_resource;
	} release

	return ret;
}



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
