# Progress report 2 (2019-11-16)

Attempting to implement my goal has helped me think more clearly about
what I am trying to do.

# Goal (restated)

Spawn _multiple_ programs in the same address-space with a heap that
is _default non-shared_. I call these entities process-as-threads.

Running Python as process-as-threads will allow me to run existing
Python programs at the speed of threads with true-parallelism,
effectively bypassing the GIL, while maintaining correctness.

Memory-safe programs never derive a pointer to memory they did not
first allocate. Therefore, multiple memory-safe programs in the same
address-space will not interfere with each other, as long as the
memory-allocator is thread-safe. The heap can be _logically
partitioned_ (each memory location is owned by one process-as-threads
or the other) without being _physically partitioned_ (in separate
address-spaces). The same can be said for the file-descriptor table. A
program will never read a file-descriptor it didn't open. Programming
with processes-as-threads is most appropriate for existing mature
projects can be safely assumed as memory-safe, such as the CPython
interpreter.

- Benefit: Using the same address-space, file-descriptor table, and
  other per-process resources reduces context-switching time, because
  the TLB, PTE, and D-cache need not be flushed.
  
- Caveat: While some of this sharing (file-descriptor table) can
  already be realized with `clone` syscall, but I think the most
  critical one is address-space sharing, because that is what
  ultimately extends the reach TLB, PTE, and data-cache.

- Benefit: Explicit shared memory can be used for fast communication
  and synchronization.
  
- Caveat: This can already be realized with `mmap`.  Write a
  thread-safe `malloc` (see SuperMalloc, jemalloc, tcmalloc)
  replacement that uses `mmap` on a shared page, and then `LD_PRELOAD`
  it to replace libc's `malloc`.

- Benefit: Using the same address-space reduces fragmentation incurred by
  hugepages (each process-as-thread's heaps can be on one hugepage).

- Main benefit: I could run existing programs written with
  multiprocessing (e.g. Python scripts) with true thread parallelism
  without code change. I am not aware of anything that does this with
  mainstream CPython. This is my 'killer app'.
  
- Extensions: The system is does not apply to only Python, but any two
  programs that need to communicate or synchronize (applications that
  use multiple languages for example).

## How much is shared

In order to maintain correctness for each process-as-thread, each
process-as-thread should have their own static and bss segments.

In general, I want to support running completely different programs in
the same address-space (JVM + Python), for example. Therefore they
need different text segments. Even in the special case of running
clones of the same program, each process-as-thread's text needs to
refer to its own copy of the static and bss, so it still must be
duplicated.

In order to execute in parallel, they should have their own stack
segments.

However they can share a heap because correct memory-safe programs
will not interfere here, as long as the memory-allocator is
thread-safe.

While `fork` can share common initialization, by moving code above the
fork-point, this is incompatible with the process-as-threads model
where processes could be from different program-images. Even if they
were from the same image, the static, bss, and heap segments would
still have to be different, so they would have to do initialization
per process-as-thread anyway.

## Default-shared vs default-nonshared memory programming models

Threads use default-shared memory: sharing is the default. While
processes-as-threads run at the speed of threads, it does not use the
same programming model. Process-as-threads is designed to run code
written as different processes, with no shared memory, so
reinterpreting them as sharing memory is incorrect.

Process-as-threads use default-non-shared shared memory: not-sharing
is the default. Existing programs still work, because their usual
memory accesses are interpreted as not sharing any memory.

But we don't want to totally disable sharing, because then there would
be no fast communication. I can implement a library that replaces IPC
with fast in-address-space copying by explicitly creating shared
data. This is analagous to `fork+mmap`. One process-as-thread can send
the address of a data-structure it would like to share to the other,
instead of sending the whole data-structure. Since they are running in
the same address-space, the other process-as-thread can natively
find and manipulate the data-structure.

# API

`clone/fork/pthread_run` are not appropriate either because it would
refer to a procedure in the same text segment, but they need to have
different text segments.

I will write a new executable called `exec_sharing` where each
arguments is a different program to run in the same address-space.

## What would programming for `exec_sharing` look like in C/C++?

You would assume the other program is already spawned in another
process or process-as-thread, and communicate with regular IPC.

    // file1.c
    int main() {
        do_stuff();
        result = talk_to_other_proc();
        // ...
    }

    // file2.c
    int main() {
        do_stuff();
        send_to_other_proc(result);
        // ...
    }

Then you can run as many programs as you want with `exec_sharing`,
using the escaped-semicolon as a separator between programs. This way
you can pass argument flags to both.

    $ exec_sharing file1.exe -v \; file2.exe -p

They'll run as if you did:

    $ ./file1.exe -v &
	$ ./file2.exe -p &
    $ wait

## What would programming for `exec_sharing` look like in Python?

That even though this is running like threads, the programming-model
is like processes, so you can use the `multiprocessing` library
transparently.

    # main.py
    import multiprocessing

    pool = multiprocessing.Pool(4)

    result = pool.map(worker_fn, data)

This would also be run by its own executable.

    $ python_sharing main.py

The execution would be as if you ran it with processes, but faster.

    $ python main.py
