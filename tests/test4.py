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

queue = libpat.Queue()
queue.push(45)
queue.push('hello')
assert(queue.try_pop() == 45)
assert(queue.pop() == 'hello')
assert(queue.try_pop() is None)
