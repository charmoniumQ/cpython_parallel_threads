import sys
import multiprocessing
n = int(sys.argv[1])
m = int(sys.argv[2])

def count(k):
    for _ in range(k):
        pass

multiprocessing.Pool().map(count, [n // m] * m)
