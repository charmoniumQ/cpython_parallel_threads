obj-m += execves.o
LINUX := ../linux-with-syscall-stubs
CFLAGS := -g -0g

modules:
	$(MAKE) -C $(LINUX) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX) M=$(PWD) clean
