# I think if Python caches libraries, that's ok.

# But if the OS caches .so files, and Python tries to import the same
# .so twice, that's bad because the second time that Python dlopen's
# the .so, the OS doesn't run init, which means .so doesn't return a module. That causes:

#   File "tests/test4.py", line 12, in <module>
# SystemError: initialization of libpat did not return an extension module

# Mind you, "import libpat" already correctly imported once in another thread
# so this is not a problem within libpat.

# To get around this, I must dupe the OS into thinking we have a different .so
# I can do this by just renaming

def exec_in_isolated_namespace():
    from pathlib import Path
    import shutil
    import tempfile
    import contextlib
    import importlib
    import random
    import sys

    @contextlib.contextmanager
    def restore_sys_path():
        old_sys_paths = sys.path[:]
        yield
        sys.path.clear()
        sys.path.extend(old_sys_paths)

    libpat_path = Path(__file__).parent / 'libpat.so'

    # go to a different directory for each thread
    with tempfile.TemporaryDirectory() as tmp_dir:

        # copy same libpat
        new_libpat_path = Path(tmp_dir) / libpat_path.name
        shutil.copy(libpat_path, new_libpat_path)

        # import THIS new libpat
        with restore_sys_path():
            del sys.path[0]
            sys.path.insert(0, str(new_libpat_path.parent))
            print(sys.path)
            print(new_libpat_path)
            libpat_mod = importlib.import_module('libpat')
            return libpat_mod

# This exports only the names I want.
libpat = exec_in_isolated_namespace()
