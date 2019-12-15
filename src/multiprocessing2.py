from typing import (
    Optional, Iterable, TypeVar, Callable, Sequence, cast, Sized, Collection, Any
)
import itertools
import contextlib
import logging
import importlib
import multiprocessing
import copy

# import cloudpickle

from libpat2 import libpat

logger = logging.getLogger('multiprocessing2')
logger.setLevel(logging.INFO)
T = TypeVar('T')
U = TypeVar('U')

dumps = copy.deepcopy
loads = lambda x: x

class Process:
    def __init__(self, in_queue, out_queue) -> None:
        self.cmd_queue = libpat.Queue()
        self.pat = libpat.PythonProcessAsThread(['', '-c',
f'''
import sys
print(sys.path)
import importlib
importlib.invalidate_caches()
import multiprocessing2
multiprocessing2.worker_main({in_queue.handle}, {out_queue.handle}, {self.cmd_queue.handle})
''',
        ])

class Pool:
    def __init__(self, processes: Optional[int] = None) -> None:
        self.closed = False
        self.in_queue = libpat.Queue()
        self.out_queue = libpat.Queue()
        if processes is None:
            processes = multiprocessing.cpu_count()
            logger.debug(f'nproc = %s', multiprocessing.cpu_count())
        self.pats: Sequence[Process] = [
            Process(self.in_queue, self.out_queue)
            for _ in range(processes)
        ]

    def close(self) -> None:
        if not self.closed:
            exit_dump = dumps('exit')
            for pat in self.pats:
                pat.cmd_queue.push(exit_dump)
            self.closed = True

    def __enter__(self) -> 'Pool':
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> bool:
        self.close()
        return False

    def __del__(self) -> None:
        # Don't depend on this
        # might not get called
        self.close()

    @contextlib.contextmanager
    def _workers_do_cmd(self, cmd: Any) -> None:
        cmd_dump = dumps(cmd)
        for pat in self.pats:
            pat.cmd_queue.push(cmd_dump)
        yield
        stop_dump = dumps('stop')
        for pat in self.pats:
            pat.cmd_queue.push(stop_dump)

    # shamelessly swiped from Lib/multiprocessing.py
    def map(
            self,
            func: Callable[[T], U],
            iterable: Collection[T],
            chunksize: int = 1
    ) -> Iterable[U]:
        assert not self.closed

        # start workers
        # I'll do this first so that they can be initializing
        # while I am setting things up
        with self._workers_do_cmd(('map', func)):

            # default value for chunksize
            if chunksize is None:
                chunksize, extra = divmod(len(iterable), len(self._pool) * 4)
                if extra:
                    chunksize += 1

            # put work into chunks and chunks into queue
            total_no = 0
            for no, data_chunk in zip(
                    itertools.count(),
                    chunk(iterable, chunksize),
            ):
                self.in_queue.push(dumps((no, data_chunk)))
            total_no = no

            return flatten1(
                chunk
                for no, chunk in sorted(
                        # when I get the results back
                        # they are unsorted
                        loads(self.out_queue.pop())
                        for _ in range(total_no)
                )
            )


def chunk(it: Iterable[T], size: int) -> Iterable[Sequence[T]]:
    '''chunk input into size or less chunks

shamelessly swiped from Lib/multiprocessing.py:Pool._get_task'''
    it = iter(it)
    while True:
        x = list(itertools.islice(it, size))
        if not x:
            return
        yield x

def flatten1(list_of_lists: Iterable[Iterable[T]]) -> Iterable[T]:
    '''flatten one level: list of lists -> list'''
    return itertools.chain.from_iterable(list_of_lists)

def worker_main(in_queue_re, out_queue_re, cmd_queue_re) -> None:
    in_queue = libpat.Queue.from_handle(in_queue_re)
    out_queue = libpat.Queue.from_handle(out_queue_re)
    cmd_queue = libpat.Queue.from_handle(cmd_queue_re)

    def map(func):
        while True:
            num_data_chunk = in_queue.try_pop()
            if num_data_chunk:
                num, data_chunk = loads(num_data_chunk)
                result_chunk = list(map(func, data_chunk))
                out_queue.push(dumps((num, data_chunk)))
            else:
                cmd = loads(cmd_queue.try_pop())
                if cmd[0] == 'stop':
                    return

    while True:
        cmd = loads(cmd_queue.pop())
        if cmd[0] == 'exit':
            break
        else:
            eval(cmd[0])(*cmd[1:])
