# FastestLocklessQueue
    This is a robust lockless queue used in multi processes through shared memory. <br>
    It can deal with the situation that any programs abort at any line.<br>
    It's not the fastest lockless queue but the most robust one.<br>
    It's called FastedLocklessQueue because it's hard to change the name in Github. (=・ω・=)<br>
    Welcome to show your better ideas.<br>

##Usage
###1. C
```
#include "queue.h"

int main()
{
    LFQueue_create(1234, 64, 4096, true);   // Create a LFQueue.
    
    LFQueue *queue = LFQueue_open(1234);    // Open a LFQueue.
    
    char buf[64];
    const char *data = "hello, world!";
    LFQueue_push(queue, data, strlen(data) + 1, NULL);
    LFQueue_pop(queue, buf, NULL);
    printf("%s\n", buf);
    
    LFQueue_close(queue);
    LFQueue_destroy(1234);
}
```

###2. Python
```
import pylfq

pylfq.create(1234, 64, 4096, True)

mq = pylfq.MQ(1234)

msg = b'hello, world!'
msg_out = bytes(64)

pylfq.push(msg)
pylfq.pop(msg_out)

print(msg_out)

pylfq.destroy(1234)
```
##Build
###C
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
###Python
```
cd python/pylfq
python setup.py build_ext --inplace  # or python setup.py install
```