from pathlib import Path
import sys
import libpat
m = int(sys.argv[1])

procs = [
    libpat.PythonProcessAsThread([
        'python',
        '-c',
        f'pass'
    ])
    for _ in range(m)
]
for proc in procs:
    proc.wait(float('inf'))
