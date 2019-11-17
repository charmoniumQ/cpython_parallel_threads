# Attempt 1 (broken): Hack `execve` syscall

At first, I was trying to modify the `execve` syscall to load and
execute program into an existing address-space. Then I could use
`pthread+execve` to implement `exec_sharing`.

## Status

[Here][1] is that branch. I have been stopped on [this segfault][3]
that I don't understand.

I don't think this will work even in theory because the
[ELF-specification][4] requires that certain segments be loaded at
_specific_ points in virtual-address-space.

If I continue by this method, I will have to use relocatable
position-independent code. While I was working on this, I realized
that there are user-space ways of doing this, and user-space methods
are preferred to kernel-space ones.

# Attempt 2 (working prototype): Use dynamic library loading

Now, I am trying to use [`dlopen/dlsym/dlclose`][5] to dynamically load the
executable (as if it were a shared-library), find the main method, and
run it in its own thread. This does in user-space what I was trying to
do in kernel-space. The only catch is the programs have to be
recompiled as shared-libraries. This is easy, because one file can be
both a shared-library and an executable (it just needs to export a
main method symbol).

## Status

[Here][2] is that branch. I have a working prototype
(`src/exec_sharing.cc`) that can already run simple C/C++
processes-as-threads.

### Usage

For example one could write:

    /* prog1.c
     * For exact deatils, see
     * https://github.com/charmoniumQ/exec_sharing/blob/c1b4e93f71eaa45652e90979b4b8e6f05dc76d69/tests/test2.cc
     * https://github.com/charmoniumQ/exec_sharing/blob/c1b4e93f71eaa45652e90979b4b8e6f05dc76d69/tests/test2.sh
     */
    int main() {
        srand(0);
        printf("rand=%d, pid=%d tid=%d rand=%d\n", (int)(rand()%100), getpid(), gettid());
        return 0;
    }

Make sure to compile it with `-shared -rdynamic -fpic`. Then run

    $ exec_sharing prog1 \; prog1

You should see the *same* `rand`, *same* `pid`, but *different* `tid`,
indicating this code which is being run in the same process as a
thread transparently. This also tests that one `rand()` does not
influence the other `rand()`; the static state is different for each
process-as-thread.

The escaped-semicolon separates the programs you want to run, so that
it knows where the arguments to `prog1` stop and the invocation of
`prog2` starts. The prototype supports arbitrarily many
processes-as-threads.

You could even write a completely different program and run it in a
process-as-thread:

    $ exec_sharing prog1 --args-go-here \; different_prog --more-args \; third_program --args

### ctors/dtors

It even works with the `init/fini` ELF segments and `ctors/dtors`
segments. The desired behavior is that the initialization code be run
once per process-as-thread, so the execution maintains corretness with
separate copies of the data in one virtual address-space. There's one
hiccup: `dlopen` sometimes caches libraries and doesn't re-initialize
when they are loaded again. To work around this, I simply copy the
library to a different filename.

    $ cp ctors_prog ctors_prog2
    $ # now we have 'tricked' dlopen/dlclose into running init/fini twice
	$ # when I load them in exec_sharing
    $ exec_sharing ctors_prog \; ctors_prog2
    ctor code
    ctor code
    dtor code
    dtor code

### Python

Instead of recompiling all of Python with `-fpic`, I am just using its
API which is already compiled as a shared library. I just need to
initialize the interpreter and call `PyRun_SimpleFile(fp, path)`.

Unfortunately this still does not work. When I run one instance in a
process-as-thread, its fine, but two does not work.


    $ exec_sharing run_python script.py
    <output of script.py>

    $ # I have to use the cp trick to reinitialize
    $ cp run_python run_python_2
    $ exec_sharing run_python script.py \; run_python_2 script.py
    [1]    26159 segmentation fault

I will spend the next segment of time debugging this problem.

# Attempt 3 (potentially): Concat elf

If `dlopen/dlsym` (attempt 2) does not work, I have a fallback method:
I can try concatenating the ELF sections themselves and then running
that. Instead of hacking the loader (as in attempt 1), I can just hack
its input to make it do what I want.

I would still have to use position-independent code (much like attempt
2), but I already have the infrastructure for that.

One downside to this method is that you would need to know how many
processes-as-threads you want _statically_ whereas `dlopen` can
_dynamically_ add new processes-as-threads during execution.

[1]: https://github.com/charmoniumQ/exec_sharing/tree/ee54187a161e1df5afcd41175ca7ac430cf977c0
[2]: https://github.com/charmoniumQ/exec_sharing/tree/c1b4e93f71eaa45652e90979b4b8e6f05dc76d69
[3]: https://github.com/charmoniumQ/exec_sharing/tree/ee54187a161e1df5afcd41175ca7ac430cf977c0/results/log#L376
[4]: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
[5]: https://linux.die.net/man/3/dlopen
