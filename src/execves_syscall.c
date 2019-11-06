#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/binfmts.h>
#include <linux/fs.h>
#include <linux/sched/task.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/sched/numa_balancing.h>
#include <linux/sched/mm.h>
#include <linux/tsacct_kern.h>
#include "execves.h"

MODULE_LICENSE("Dual MIT/GPL");

#define acquire {
#define use do
#define release while (0);
#define end_resource }
#define cleanup { printk(KERN_DEBUG "error_line %ld\n", error_line); break; }

// symbols from fs/exec.c

SYSCALL_DEFINE4(execves,
		const char __user *, filename,
		const char __user * const __user *, _argv,
		const char __user * const __user *, _envp,
		const execves_attr_t __user *, attr
) {
	unsigned long ret = 0;
	unsigned long error_line = 0;
	execves_attr_t attr_;
	struct user_arg_ptr argv = { .ptr.native = _argv };
	struct user_arg_ptr envp = { .ptr.native = _envp };

	printk(KERN_DEBUG "execves: entering\n");

	use {

		acquire struct filename* filename_ = getname(filename);
		if (IS_ERR(filename_)) {
			ret = PTR_ERR(filename_);
			cleanup;
		}
		use {

			printk(KERN_DEBUG "execves: loading %s\n", filename_->name);

			while (unlikely((ret = copy_from_user(&attr_, attr, sizeof(attr_))) > 0));
			if (unlikely(ret < 0))
				cleanup;

			if ((current->flags & PF_NPROC_EXCEEDED) &&
				atomic_read(&current_user()->processes) > rlimit(RLIMIT_NPROC)) {
				ret = -EAGAIN;
				cleanup;
			}

			acquire struct linux_binprm *bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
			if (!bprm) { ret = -ENOMEM; cleanup; }
			use {

				ret = prepare_bprm_creds(bprm);
				if (ret) cleanup;

				check_unsafe_exec(bprm);

				acquire current->in_execve = 1;
				use {

					struct file* file = do_open_execat(AT_FDCWD, filename_, 0);
					if (IS_ERR(file)) { ret = PTR_ERR(file); cleanup; }

					sched_exec();

					bprm->file = file;
					if (!filename_) {
						bprm->filename = "none";
					} else {
						bprm->filename = filename_->name;
					}
					bprm->interp = bprm->filename;

					acquire ret = bprm_mm_init(bprm);
					use {

						if (ret) cleanup;

						ret = prepare_arg_pages(bprm, argv, envp);
						if (ret < 0) cleanup;

						ret = prepare_binprm(bprm);
						if (ret < 0) cleanup;

						ret = copy_strings_kernel(1, &bprm->filename, bprm);
						if (ret < 0) cleanup;

						bprm->exec = bprm->p;
						ret = copy_strings(bprm->envc, envp, bprm);
						if (ret < 0) cleanup;

						ret = copy_strings(bprm->argc, argv, bprm);
						if (ret < 0) cleanup;

						would_dump(bprm, bprm->file);

						ret = exec_binprm(bprm);
						if (ret < 0) cleanup;

						ret = 0;

					} release {
						if (ret != 0 && bprm->mm) {
							acct_arg_size(bprm, 0);
							mmput(bprm->mm);
						}
					} end_resource;

				} release {
					current->fs->in_exec = 0;
					current->in_execve = 0;
				} end_resource;

				if (ret == 0) {
					rseq_execve(current);
					acct_update_integrals(current);
					task_numa_free(current, false);
				}
				printk(KERN_DEBUG "execves: exiting\n");

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
