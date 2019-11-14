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
#include <linux/mm.h>
#include <linux/tsacct_kern.h>
#include <asm/mmu_context.h>
#include <asm/mmu.h>
#include <linux/cn_proc.h>
#include "execves.h"

MODULE_LICENSE("Dual MIT/GPL");

#define acquire {
#define use do
#define release while (0);
#define end_resource }
#define cleanup break

// linux/arch/x86/include/asm/mpx.h
#define MPX_INVALID_BOUNDS_DIR	((void __user *)-1)

// linux/fs/exec.c:bprm_mm_init
int bprm_mm_init_sh(struct linux_binprm *bprm) {
	int ret = 0;
	struct mm_struct *mm = NULL;

	bprm->mm = mm = current->mm;
	/* bprm->mm = mm = mm_alloc(); */

	/* Save current stack limit for all calculations made during exec. */
	task_lock(current->group_leader);
	bprm->rlim_stack = current->signal->rlim[RLIMIT_STACK];
	task_unlock(current->group_leader);

	// linux/fs/exec.c:__bprm_mm_init
	use {

		acquire struct vm_area_struct *vma = bprm->vma = vm_area_alloc(mm);
		if (!vma) {
			ret = -ENOMEM;
			cleanup;
		}
		use {

			vma_set_anonymous(vma);

			if (down_write_killable(&mm->mmap_sem)) {
				ret = -EINTR;
				cleanup;
			}

			acquire;
			use {
				/*
				 * Place the stack at the largest stack address the architecture
				 * supports. Later, we'll move this to an appropriate place. We don't
				 * use STACK_TOP because that can depend on attributes which aren't
				 * configured yet.
				 */
				BUILD_BUG_ON(VM_STACK_FLAGS & VM_STACK_INCOMPLETE_SETUP);
				vma->vm_end = STACK_TOP_MAX;
				vma->vm_start = vma->vm_end - PAGE_SIZE;
				vma->vm_flags = VM_SOFTDIRTY | VM_STACK_FLAGS | VM_STACK_INCOMPLETE_SETUP;
				vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

				ret = insert_vm_struct(mm, vma);
				if (ret) cleanup;

				mm->stack_vm = mm->total_vm = 1;
				// linux/arch/x86/include/asm/mmu_context.h:arch_bprm_mm_write
				// linux/arch/x86/include/asm/mpx.h:mpx_mm_init
#ifdef CONFIG_X86_INTEL_MPX
				mm->context.bd_addr = MPX_INVALID_BOUNDS_DIR;
#endif
				bprm->p = vma->vm_end - sizeof(void *);

			} release {
				up_write(&mm->mmap_sem);
			} end_resource;

		} release {
			if (ret != 0) {
				bprm->vma = NULL;
				vm_area_free(vma);
			}
		} end_resource;
	} release;

	return ret;
}

// fs/exec.c:exec_binprm
int exec_binprm_sh(struct linux_binprm *bprm) {
	pid_t old_pid, old_vpid;
	int ret;

	/* Need to fetch pid before load_binary changes it */
	old_pid = current->pid;
	rcu_read_lock();
	old_vpid = task_pid_nr_ns(current, task_active_pid_ns(current->parent));
	rcu_read_unlock();

	ret = search_binary_handler(bprm);
	// TODO: This stuff for logging, tracing, and accounting
	/* if (ret >= 0) { */
		/* audit_bprm(bprm); */
		/* trace_sched_process_exec(current, old_pid, bprm); */
		/* ptrace_event(PTRACE_EVENT_EXEC, old_vpid); */
		/* proc_exec_connector(current); */
	/* } */

	return ret;
}

SYSCALL_DEFINE4(execves,
		const char __user *, filename,
		const char __user * const __user *, _argv,
		const char __user * const __user *, _envp,
		const execves_attr_t __user *, attr
) {
	unsigned long ret = 0;
	execves_attr_t attr_;
	// linux/fs/exec.c:SYSCALL_DEFINE3(execve)
	// linux/fs/exec.c:do_execve
	struct user_arg_ptr argv = { .ptr.native = _argv };
	struct user_arg_ptr envp = { .ptr.native = _envp };

	printk(KERN_DEBUG "execves: entering\n");

	// linux/fs/exec.c:do_execveat_common
	// linux/fs/exec.c:__do_execve_file
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

			// TODO: checking security
			/* if ((current->flags & PF_NPROC_EXCEEDED) && */
			/* 	atomic_read(&current_user()->processes) > rlimit(RLIMIT_NPROC)) { */
			/* 	ret = -EAGAIN; */
			/* 	cleanup; */
			/* } */

			acquire struct linux_binprm *bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
			if (!bprm) { ret = -ENOMEM; cleanup; }
			use {

				 // TODO: copy creds from current
				ret = prepare_bprm_creds(bprm);
				if (ret) cleanup;

				// TODO: checking security
				/* check_unsafe_exec(bprm); */

				acquire current->in_execve = 1;
				use {

					struct file* file = do_open_execat(AT_FDCWD, filename_, 0);
					if (IS_ERR(file)) { ret = PTR_ERR(file); cleanup; }

					// TODO: load balancing
					/* sched_exec(); */

					bprm->file = file;
					if (!filename_) {
						bprm->filename = "none";
					} else {
						bprm->filename = filename_->name;
					}
					bprm->interp = bprm->filename;

					acquire ret = bprm_mm_init_sh(bprm);
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

						bprm->sharing = 1;
						ret = exec_binprm_sh(bprm);
						if (ret < 0) cleanup;

						ret = 0;

					} release {
						if (ret != 0 && bprm->mm) {
							 // TODO: This stuff for logging, tracing, and accounting 
							/* acct_arg_size(bprm, 0); */
							/* mmput(bprm->mm); */
						}
					} end_resource;

				} release {
					current->fs->in_exec = 0;
					current->in_execve = 0;
				} end_resource;

				if (ret == 0) {
					 // TODO: This stuff for logging, tracing, and accounting 
					/* rseq_execve(current); */
					/* acct_update_integrals(current); */
					/* task_numa_free(current, false); */
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
