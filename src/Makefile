obj-m += execves.o
LINUX := ../linux-with-syscall-stubs
CFLAGS := -g -0g
KASAN_SANITIZE_execves.o := n

modules:
	$(MAKE) -C $(LINUX) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX) M=$(PWD) clean
