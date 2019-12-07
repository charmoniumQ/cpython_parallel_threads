import sys
sys.path.insert(0, 'src')
import multiprocessing_sharing

import multiprocessing


def f(x):
    return x*x


if __name__ == '__main__':
    multiprocessing.set_start_method('process_as_thread')
    with multiprocessing.Pool() as p:
        print(p.map(f, [1, 2, 3]))

#  ./src/run_python2.exe tests/test5.py
# Traceback (most recent call last):
#   File "tests/test5.py", line 14, in <module>
#     with multiprocessing.Pool() as p:
#   File "/usr/lib/python3.7/multiprocessing/context.py", line 117, in Pool
#     from .pool import Pool
#   File "/usr/lib/python3.7/multiprocessing/pool.py", line 17, in <module>
#     import queue
#   File "/usr/lib/python3.7/queue.py", line 16, in <module>
#     from _queue import Empty
# ImportError: /usr/lib/python3.7/lib-dynload/_queue.cpython-37m-x86_64-linux-gnu.so: undefined symbol: _Py_NoneStruct
