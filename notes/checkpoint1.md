# Sharing Virtual Address-Space for True-threading in Python (progress report)

[Repository][1]
[Commit log][2]

## Project Build Toolchain

I have a working toolchain that:

- Builds Linux kernel with my modifications
- Bootstraps a Debian user-space
- Packs everything into a disk image
- Boots the image in QEMU with VT-x
- Runs a test-case (calls my system-call) in user-space
- Logs the results

The toolchain requires no manual keyboard interaction at all.

All the steps are cached, so minimal amount of work has to be
repeated.

## Linux Kernel Hacking

I have spent a lot of time trying to add a system-call in a kernel
module (as this guide does). Unfortunately the guide is written for
Linux 2.6, and no longer applies due to kernel address-space layout
randomization (kaslr) and other security techniques.

I tried manually exporting the symbol, disabling kaslr, using
System.map, and other techniques, but to no avail.

It seems the easiest way to add a system call is to write it directly
into the Linux kernel, instead of a kernel module. This is problematic
because it involves recompiling the whole kernel every iteration, but
it seems I have no choice.

## `exec_sharing`
The only thing I need to implement in kernel-space is an exec_sharing
syscall that loads a new program into an existing memory space. I have
begun copying the regular exec syscall (located here), but I do not
have a working prototype.

The clone syscall already can fork a process, creating a new PID but
not a new memory-space, so I don’t need to implement this.
