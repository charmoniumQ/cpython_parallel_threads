import os
import random
import threading

print(
    'pid', os.getpid(),
    'tid', threading.get_ident(),
    'rand', random.randint(0, 100),
)
