- Implement `map` and other primitives in C++ with TBB instead of
  Python.

    Right now, I use an implementation of map written in Python that
    uses a global work-queue.  TBB is more efficient because it
    implements distributed queing and work-stealing, which lifts the
    bottleneck off of the global work-queue.

- Check for memory-leaks

    Right now, I call incref and decref by hand in C++. I would like
    to validate that this is correct by searching for memory leaks.

- Lock py_DECREF in threads AND eliminate deepcopies.

    Right now, I deepcopy objects before I send them to another
    thread; This is so that every object is only referenced from
    exactly one thread. Then callin the incref/decref is safe.

	If I could somehow lock py_DECREF, but only for objects that get
    sent to other threads, then I could safely pass objects without
    deepcopying.

    Still maybe not, because there might be internal object references
    that can't be manipulated in a thread-safe way.

- Copy-free FS

    Build an VFS module or FUSE that makes mount/a/b/c map to
    /a/b. That way, the same file has different paths without having
    to copy it.

- Make work with any version of Python

    Right now, I force Python 3.7 in some places (see Makefile)

- Make portable

    Right now, I hardcode path to Python so.

- Remove mutex in src/dynamic_lib.cc. I don't think it is necessary.
