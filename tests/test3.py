import os
import random
import threading

# using 1 print call makes it less likely that the outputs will be
# mangled together from other process-as-threads
print(f'''
pid {os.getpid()}
tid {threading.get_ident()}
rand {random.randint(0, 100)}
'''.strip())
