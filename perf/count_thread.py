import sys
import threading
n = int(sys.argv[1])
m = int(sys.argv[2])

def count(k):
    for _ in range(k):
        pass

threads = [
    threading.Thread(target=count, args=(n // m,))
    for _ in range(m)
]
for thread in threads:
    thread.start()
for thread in threads:
    thread.join()
