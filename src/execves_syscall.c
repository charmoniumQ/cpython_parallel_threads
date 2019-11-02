#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/binfmts.h>
#include <linux/fs.h>
#include <linux/sched/task.h>
#include "execves.h"

MODULE_LICENSE("Dual MIT/GPL");

#define acquire {
#define use error_line++; do
#define release while (0);
#define end_resource }
#define cleanup { printk(KERN_DEBUG "error_line %ld\n", error_line); break; }

// symbols from fs/exec.c

SYSCALL_DEFINE4(execves,
		const char __user *, filename,
		const char __user * const __user *, argv,
		const char __user * const __user *, envp,
		const execves_attr_t __user *, attr
) {
	unsigned long ret = 0;
	unsigned long error_line = 0;

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
	} release;

	return ret;
}
