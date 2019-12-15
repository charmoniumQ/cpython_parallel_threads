#!/usr/bin/env python
import sys
sys.path.insert(0, 'src')
import multiprocessing2

def square(x):
    return x**2

with multiprocessing2.Pool() as pool:
    assert(pool.map(square, range(10)) == list(map(square, range(10))))
