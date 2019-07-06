# FastestLocklessQueue
中文版README请往下翻
## 1. Introduction
- This is a robust lockless queue used in multi processes through shared memory.
- It can deal with the situation that any programs abort at any line.
- It's not the fastest lockless queue but the most robust one.
- It's called FastedLocklessQueue because it's hard to change the name in Github. (=・ω・=)
- Welcome to show your better ideas.

## 2. Algorithm
- There are two LFRings and an array with the same size.
- Each element in resource ring or message ring identifies one node in the array. (subscription)
- ID -1 means invalid subscription.
- When a queue created, resource ring and message ring would be initialized like these. Resource ring was full of valid IDs, but message ring is none.
- When enqueuing to the queue, get a subscription from resource ring, modify the data in real resource array with subscription got from resource ring, and insert it into the message ring. If all of the IDs were inserted into the message ring and overwrite mode was set, pop a node from message ring, modify the data, and reinsert into the message ring, which means dequeue from the queue and enqueue it again to implement the overwrite mode.
- When dequeuing, it's the same. Get ID from message ring, copy the data out from real resource array, and insert it into the resource ring.
- All operations on getting ID from or inserting ID into are implemented with CAS.
- If you want to know why it's robust, read the code please or you could try to abort at any line. 
<br>
<br>

Resouce ID Ring<br>
---------------------------<br>
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |<br>
---------------------------<br>
<br>
Message ID Ring<br>
-------------------------------<br>
| -1 | -1 | -1 | -1 | -1 | -1 | -1 |<br>
-------------------------------<br>
<br>
Real Resource<br>
------------------------------------<br>
| LFNode | LFNode | LFNode | ... |<br>
------------------------------------<br>

## 3. Usage
#### C
```
#include "queue.h"

int main()
{
    // Create a LFQueue.
    // You can open the queue with key 1234 in another process or thread.
    LFQueue_create(1234, 64, 4096, true);   
    
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

#### Python
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
## 4. Build
#### C
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
#### Python
```
cd python/pylfq
python setup.py build_ext --inplace  # or python setup.py install
```

<br>
<br>
<br>

# FastestLocklessQueue
## 1. 介绍
- 这是一个多进程健壮的基于共享内存的无锁队列
- 它可以应对进程在任何一行代码崩溃的情况，包括在queue内部
- 欢迎提出更多有趣的想法

## 2. 算法
- There are two LFRings and an array with the same size.
- 这个队列里有三个结构，有两个LFRing，resource ring和message都不管理实际的资源，它们当中保存的是资源的下标，另外一个结构是实际资源的数组，两个LFRing中的内存都和数组的索引一一对应。
- 下标为-1表示这是一个无效的索引
- 当队列被创建时，队列将初始化为下图所示，resource ring初始时是被填满的，而message ring初始时是空的。
- 入队时，先从resource ring中弹出一个下标，如果成功获得一个有效的下标，则取数组中找到该下标对应的元素，将数据拷贝到这个元素中，然后将这个下标入队到message ring中，完成一次入队操作。如果从resource ring获取失败，则有两种情况，一是开启了覆写模式，而是非覆写模式。在覆写模式下，队列中最老的节点应该被覆盖，所以会从message ring中弹出一个下标，对该下标的内容进行覆写，然后重新将下标入队到message ring中完成覆写。若为非覆写模式，从resource ring获取失败则直接返回一个入队失败的错误值。
- 出队时同理，先从message ring出队一个下标，再把数组中索引为该下标的元素里的数据拷贝出来，然后将该下标入队到resource ring中，归还资源。
- 所有的LFRing操作均是由CAS操作实现的。
- 为什么说该队列能容忍程序在任意一行代码崩溃呢，请看阅读代码。
<br>
Resouce ID Ring<br>
---------------------------<br>
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |<br>
---------------------------<br>
<br>
Message ID Ring<br>
-------------------------------<br>
| -1 | -1 | -1 | -1 | -1 | -1 | -1 |<br>
-------------------------------<br>
<br>
Real Resource<br>
------------------------------------<br>
| LFNode | LFNode | LFNode | ... |<br>
------------------------------------<br>
