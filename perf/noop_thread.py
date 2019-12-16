import sys
import threading
m = int(sys.argv[1])

def noop():
    pass

threads = [
    threading.Thread(target=noop)
    for _ in range(m)
]
for thread in threads:
    thread.start()
for thread in threads:
    thread.join()
