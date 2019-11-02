#include <linux/fs.h>
#include <linux/binfmts.h>

int prepare_bprm_creds(struct linux_binprm *bprm);
void free_bprm(struct linux_binprm *bprm);
void check_unsafe_exec(struct linux_binprm *bprm);
struct file *do_open_execat(int fd, struct filename *name, int flags);
int bprm_mm_init(struct linux_binprm *bprm);
