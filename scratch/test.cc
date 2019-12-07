#include <dlfcn.h>

int main() {
	// void* handle = dlopen("./lib.so", RTLD_LAZY | RTLD_LOCAL);
	void* handle = dlmopen(LM_ID_NEWLM, "./lib.so", RTLD_LAZY | RTLD_LOCAL);

	// int (*lib_main)();
	// lib_main = (int (*)()) dlsym(handle, "main");
	// lib_main();

	return 0;
}
