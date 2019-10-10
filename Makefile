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
USER_SRCS := $(shell find user_src/ -type f -printf '%P\n')

$(LINUX)/.config:
	rm $@ && \
	cd $(LINUX) && \
	$(MAKE) x86_64_defconfig && \
	$(MAKE) kvmconfig && \
	cd .. && \
	patch $@ linux_debug_config.diff
# DRY defining these env vars
# pass them to my script instead

$(LINUX_IMAGE): $(LINUX)/.config
	$(MKDIR) -p build && \
	git submodule update --init $(LINUX) && \
	cd $(LINUX) && \
	$(MAKE) -j bzImage && \
	mv arch/x86/boot/bzImage ../$(LINUX_IMAGE) && \
	true

$(ROOTFS_DIR)/debootstrap: packages.txt
	sudo rm -rf $(ROOTFS_DIR) && \
	mkdir -p $(ROOTFS_DIR) && \
	sudo debootstrap --variant=minbase --arch=amd64 --include=$(shell cat packages.txt | tr \\n ,) stable $(ROOTFS_DIR) && \
	echo 'root:root' | sudo chroot $(ROOTFS_DIR) chpasswd && \
	touch $@ && \
	true
# https://unix.stackexchange.com/questions/275429/creating-bootable-debian-image-with-debootstrap/473256#473256

$(ROOTFS_DIR)/execves.ko: src/execves.c $(ROOTFS_DIR)/debootstrap
	cd src && \
	$(MAKE) modules && \
	cd .. && \
	mv src/execves.ko $@ && \
	true

$(ROOTFS_DIR)/user_srcs: $(USER_SRCS)
	cd user_src && sudo cp $(USER_SRCS) ../$(ROOTFS_DIR) && cd .. && touch $@

$(ROOTFS_IMAGE): $(ROOTFS_DIR)/execves.ko $(ROOTFS_DIR)/init.sh
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

shell: $(ROOTFS_IMAGE) $(LINUX_IMAGE)
	sudo qemu-system-x86_64 \
	    -kernel $(LINUX_IMAGE) \
	    -drive file=$(ROOTFS_IMAGE),index=0,media=disk,format=raw \
	    -nographic \
	    -m 1G \
	    -enable-kvm \
	    -append "console=ttyS0 root=/dev/sda rw single" \
	    -gdb tcp::1234 \
	    && \
	true

results/log: $(ROOTFS_IMAGE) $(LINUX_IMAGE)
	mkdir results && \
	sudo qemu-system-x86_64 \
	    -kernel $(LINUX_IMAGE) \
	    -drive file=$(ROOTFS_IMAGE),index=0,media=disk,format=raw \
	    -nographic \
	    -m 1G \
	    -enable-kvm \
	    -append "console=ttyS0 root=/dev/sda rw single" \
	    -gdb tcp::1234 | tee $@ \
	&& true

clean:
	rm -rf build && \
	cd $(LINUX) && $(MAKE) clean && cd .. && \
	cd src && $(MAKE) clean && cd .. && \
	true
