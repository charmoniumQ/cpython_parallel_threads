SHELL := /bin/sh
MKDIR := mkdir
LINUX := linux-with-syscall-stubs
LINUX_IMAGE := build/linux.bzImg
ROOTFS_IMAGE := build/rootfs.img
ROOTFS_DIR := build/rootfs
ROOTFS_MOUNT := build/rootfs.img.mount
MODULE := build/exec_sharing.ko
MODULE_SOURCE := src/main.cxx
NBD := /dev/nbd0

$(LINUX)/.config: make_linux_config.sh
	$(SHELL) $< $@ $(LINUX) $(MAKE)
# DRY defining these env vars
# pass them to my script instead

$(LINUX_IMAGE): $(LINUX)/.config
	$(MKDIR) -p build && \
	git submodule update --init $(LINUX) && \
	cd $(LINUX) && \
	$(MAKE) -j $(shell nproc) bzImage && \
	mv arch/x86/boot/bzImage ../$(LINUX_IMAGE) && \
	true

$(ROOTFS_DIR)/etc/os-release:
	mkdir -p $(ROOTFS_DIR) && \
	sudo debootstrap --variant minbase --arch amd64 stable $(ROOTFS_DIR) && \
	true

$(ROOTFS_DIR)/srv/mydata:
	mkdir -p $(ROOTFS_DIR) && \
	sudo touch $@ && \
	true

$(ROOTFS_DIR): $(ROOTFS_DIR)/etc/os-release $(ROOTFS_DIR)/srv/mydata

$(ROOTFS_IMAGE): $(ROOTFS_DIR)
	$(MKDIR) -p build && \
	qemu-img create -f raw $@ 1g && \
	mkfs.ext2 $@ && \
	sudo qemu-nbd -c $(NBD) -f raw $(ROOTFS_IMAGE) && \
	mkdir -p $(ROOTFS_MOUNT) && \
	sudo mount $(NBD) $(ROOTFS_MOUNT) && \
	sudo rsync -HSaxX $(ROOTFS_DIR)/ $(ROOTFS_MOUNT)/ && \
	sudo umount $(ROOTFS_MOUNT)
	sudo qemu-nbd -d $(NBD)
	true
# https://stackoverflow.com/a/36530204/1078199

boot: $(ROOTFS_IMAGE) $(LINUX_IMAGE)
	qemu-system-x86_64 \
	    -kernel $(LINUX_IMAGE) \
	    -drive file=$(ROOTFS_IMAGE),index=0,media=disk,format=raw \
	    -nographic \
	    -m 512M \
	    -append "console=ttyS0 root=/dev/sda" \
	    -gdb tcp::1234 \
	&& true
# -enable-kvm

clean_linux:
	cd $(LINUX) && $(MAKE) clean

clean: clean_linux
	rm -rf build
