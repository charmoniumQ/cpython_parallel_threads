import sys
import multiprocessing
m = int(sys.argv[1])

def count(x):
    pass

multiprocessing.Pool().map(count, [None] * m)
