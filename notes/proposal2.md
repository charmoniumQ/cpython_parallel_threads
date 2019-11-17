# Sharing Virtual Address-Space for True-threading in Python (proposal)
## Abstract
The OS can allocate a pages (possibly huge pages) to a group of
processes which trust each other as long as each of the processes
agree to use non-overlapping ranges. This offers much faster
context-switching (and eliminates fragmentation problems when huge
pages are used).

This OS feature could "turn processes into threads" at runtime without
any programmer intervention. In Python, programmers choose between
multiprocessing (true-parallelism but expensive start-up and
communication) and multithreading (cheap start-up and communication,
but the Global Interpreter Lock limits parallelism). However, with
this OS feature, one could instantiate multiple Python interpreters
with the context-switching speed of threads.

This does eschew security, but in big Python applications, programmers
want to use threads, indicating that they do not need security
isolation.

## Prior work

[PEP 554][2] is also an attempt to bring true-threading to Python by
core-developer Eric Snow dating back to 2015. The work is painstaking,
because when Python makes heavy use of these inherently
single-threaded abstractions: atexit handlers, signal handlers, static
variables, and others. These all have to be rewritten. The work is
[active][1], but no end is in sight. However, running multiple
processes in the same address-space could get most of this benefit
with much less work.

I don’t intend for this work to replace PEP 554. Python is supposed to
be multi-platform, so it should not depend on an OS-specific
patch. PEP 554 also presents a more efficient model, since the
interpreters can share more (static-data segment). I intend to create
a useful extension to Linux, and parallelizing Python is just one
case-study out of many possible applications.

[1]: https://github.com/ericsnowcurrently/multi-core-python/projects
[2]: https://www.python.org/dev/peps/pep-0554/
