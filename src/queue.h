#ifndef _LOCKFREEQUEUE_QUEUE_H_
#define _LOCKFREEQUEUE_QUEUE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>

#include "ring.h"


#define QUEUE_MAGIC 1709394

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
