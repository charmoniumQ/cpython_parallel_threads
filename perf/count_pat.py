from pathlib import Path
import sys
import libpat
n = int(sys.argv[1])
m = int(sys.argv[2])

procs = [
    libpat.PythonProcessAsThread([
        'python',
        '-c',
        f'for _ in range({n // m}): pass'
    ])
    for _ in range(m)
]
for proc in procs:
    proc.wait(float('inf'))
