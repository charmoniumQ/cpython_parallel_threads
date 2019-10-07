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

$(ROOTFS_DIR)/etc/os-release:
	mkdir -p $(ROOTFS_DIR) && \
	sudo debootstrap --variant=minbase --arch=amd64 --include=kmod,iputils-ping stable $(ROOTFS_DIR) && \
	true

$(ROOTFS_DIR)/execves.ko: src/execves.c
	cd src && \
	$(MAKE) modules && \
	cd .. && \
	mv src/execves.ko $@ && \
	true

$(ROOTFS_DIR)/init.sh: user_src/init.sh
	cp $< $@

$(ROOTFS_DIR): $(ROOTFS_DIR)/etc/os-release $(ROOTFS_DIR)/execves.ko $(ROOTFS_DIR)/init.sh

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
	sudo qemu-system-x86_64 \
	    -kernel $(LINUX_IMAGE) \
	    -drive file=$(ROOTFS_IMAGE),index=0,media=disk,format=raw \
	    -nographic \
	    -m 1G \
	    -enable-kvm \
	    -append "console=ttyS0 root=/dev/sda rw init=/init.sh" \
	    -gdb tcp::1234 \
	&& true

clean:
	rm -rf build && \
	cd $(LINUX) && $(MAKE) clean && cd .. && \
	cd src && $(MAKE) clean && cd .. && \
	true
