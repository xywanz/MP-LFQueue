#ifndef _LOCKFREEQUEUE_QUEUE_H_
#define _LOCKFREEQUEUE_QUEUE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>

#include "ring.h"


#define QUEUE_MAGIC 1709394

/*
        LFQueue的节点数据结构，在一个LFQueue中，所有的LFNode均为相同长度，LFNode存储的
    数据大小有限制，即max_size，max_size应该64的倍数，在初始化时函数会自动向上取64的倍数。
    size表示实际数据长度，size <= max_size。
 */
typedef struct
{
    uint32_t size;
    char data[] CACHE_ALIGNED;
} LFNode;

typedef struct
{
    uint64_t        magic;
    uint64_t        node_data_size;
    uint32_t        node_count;
    uint64_t        node_total_size;

    volatile bool   pause;
    int             key;

    char            aligned[] CACHE_ALIGNED;
} LFHeader;

/*
        LFQueue是一个基于共享内存的、多进程无锁的、存储定长数据的循环队列。
    程序在任意一行崩溃都不会影响该队列的正常运行。但会导致节点的丢失，相当于
    内存泄漏，可通过定期对nodes进行检查或通过命令行手动恢复。
        LFQueue基于LFRing，LFQueue中有两个Ring，一个用于管理可用节点的下标，
    另一个用于管理已入队的节点的下标。实际的节点存放于nodes中。入队有两种模式：
        在非覆盖模式中，入队时，先尝试从Resource Ring出队获取一个可用的节点下标，
    若无可用节点，则返回一个负值，若成功获取节点，将数据拷贝至节点后，将节点下标入
    队至Node Ring，即完成入队。
        在覆盖模式中，入队时，尝试从Resource Ring出队获取一个可用的节点下标，若无
    可用节点下标，则从Node Ring中出队一个节点（即最旧的可读节点），若获取失败，则
    返回一个负值（从两个队列都获取失败是正常现象，因为有多个进程在读写，重复调用入队
    即可），若成功获取，将数据拷贝至节点后，将节点下标入队至Node Ring，即完成入队。
        出队时，从Node Ring出队一个节点，若无可读节点，则返回一个负值，若有，则
    把数据拷贝完成后，将节点下标入队至Resource Ring，表示归还资源，完成出队。
        若在出队入队之间程序崩溃，会导致节点的丢失，但不会影响队列的正常运行。使用
    者们也可自行分析，一次入队或出队需要4-6次原子操作，在这几个原子操作之间崩溃，会
    使得队列崩溃吗？
 */
typedef struct
{
    LFHeader *header;
    LFRing *resc_ring;
    LFRing *node_ring;
    char *nodes;
} LFQueue;


int LFQueue_create(int key, uint64_t data_size, uint32_t count);
int LFQueue_destroy(int key);
void LFQueue_reset(LFQueue *queue);
LFQueue *LFQueue_open(int key);
void LFQueue_close(LFQueue *queue);
void LFQueue_pause(LFQueue *queue);
void LFQueue_resume(LFQueue *queue);
void LFQueue_print(LFQueue *queue);

int LFQueue_push(LFQueue *queue, LFNode *node, bool overwrite);
int LFQueue_pop(LFQueue *queue, LFNode *node);

#endif
