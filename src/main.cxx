#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>

#define __NR_execves 436

int execves(const char *pathname, char *const argv[], char *const envp[], const execves_attr_t* attr) {
    syscall(__NR_execves, pathname, argv, envp, attr);
}

int main(int argc, char * const argv[]) {
    int clone_flags = 0
	| CLONE_FILES // no brainer; fd table is 'partitioned'
	| CLONE_FS // todo: investigate consequences
	| CLONE_IO // todo: investigate consequences
	// | CLONE_NEWCGROUP // conainter
	// | CLONE_NEWIPC // conainter
	// | CLONE_NEWNET // conainter
	// | CLONE_NEWNS // conainter
	// | CLONE_NEWPID // conainter
	// | CLONE_NEWUSER // conainter
	// | CLONE_NEWUTS // conainter
	// | CLONE_PARENT // no brainer; parent PID should be set to this processes
	// | CLONE_PID // no brainer; use same PID is insane
	| CLONE_PTRACE // no brainer; this makes ptrace work properly
	// | CLONE_SETTLS
	// | CLONE_SIGHAND // todo: investigate consequences
	// | CLONE_SYSVSEM // no brainer; needed for correctness (semaphores give true mutual exclusion)
	// | CLONE_THREAD // no brainer; allows these guys to have their own threads
	// | CLONE_UNTRACED // no brainer; allows ptrace to work
	// | CLONE_VFORK // no brainer; I don't want to give away the virt mem space
	CLONE_VM
	;
    // execvp
    execves(nullptr, nullptr, nullptr, nullptr);
    return 0;
}
