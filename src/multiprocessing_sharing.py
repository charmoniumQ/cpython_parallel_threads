import os
from libpat import PythonProcessAsThread
from multiprocessing import util, process
from multiprocessing.context import (
    reduction, set_spawning_popen, _concrete_contexts, BaseContext
)
from multiprocessing import spawn


# TODO: hook in with resource_tracker

class Popen(object):
    method = 'process_as_thread'

    def __init__(self, process_obj):
        util._flush_std_streams()
        self._fds = []
        self.ppat = None
        self._launch(process_obj)
        # don't call super

    def poll(self, flag=os.WNOHANG):
        if self.ppat:
            if flag | os.WNOHANG:
                return self.ppat.wait(0)
            else:
                return self.ppat.wait(float('inf'))

    def wait(self, timeout=0):
        if self.ppat:
            return self.ppat.wait(timeout)

    def _launch(self, process_obj):
        from multiprocessing import resource_tracker
        tracker_fd = resource_tracker.getfd()
        self._fds.append(tracker_fd)

        prep_data = spawn.get_preparation_data(process_obj._name)
        fp = io.BytesIO()
        set_spawning_popen(self)
        try:
            reduction.dump(prep_data, fp)
            reduction.dump(process_obj, fp)
        finally:
            set_spawning_popen(None)

        paren_r = child_w = child_r = paren_w = None
        try:
            paren_r, child_w = os.pipe()
            child_r, paren_w = os.pipe()
            cmd = spawn.get_command_line(tracker_fd=tracker_fd,
                                         pipe_handle=child_r)
            if 'python' in cmd[0]:
                cmd[0] = './src/run_python2.so'
            self._fds.extend([child_r, child_w])

            self.ppat = PythonProcessAsThread(cmd)

            with open(paren_w, 'wb', closefd=False) as f:
                f.write(fp.getbuffer())
        finally:
            fds_to_close = []
            for fd in (paren_r, paren_w, child_r, child_w):
                if fd is not None:
                    fds_to_close.append(fd)
            self.finalizer = util.Finalize(self, util.close_fds, fds_to_close)


class ProcessAsThreadProcess(process.BaseProcess):
    _start_method = 'process_as_thread'
    @staticmethod
    def _Popen(process_obj):
        return Popen(process_obj)


class ProcessAsThreadContext(BaseContext):
    _name = 'process_as_thread'
    Process = ProcessAsThreadProcess


_concrete_contexts['process_as_thread'] = ProcessAsThreadContext()
