import sys
sys.path.insert(0, 'src')

import libpat
procs = [
    libpat.ProcessAsThread(['./tests/test1_a.so']),
    libpat.ProcessAsThread(['./tests/test1_b.so']),
    libpat.PythonProcessAsThread(['python', '-c', 'print(248)']),
    libpat.PythonProcessAsThread(['python', '-c', 'print(358)']),
]
assert all(proc.wait(1.0) == 0 for proc in procs)
