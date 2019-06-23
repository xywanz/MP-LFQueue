#ifndef _LOCKFREEQUEUE_RING_H_
#define _LOCKFREEQUEUE_RING_H_

#include <stdint.h>
#include <stdatomic.h>

#define CACHE_ALIGNED   __attribute__((aligned(64)))

#define LFRING_INVALID_ID           0xffffffffUL
#define LFRING_MAX_BUSYLOOP_COUNT   16
#define LFRING_MASK_LOW32           0xffffffffUL

#ifdef __cplusplus
extern "C" {
#endif

static __always_inline uint32_t power_of_two(uint32_t x)
{
    return 0;
}

typedef struct
{
    uint64_t node_count CACHE_ALIGNED;
    uint64_t node_count_mask CACHE_ALIGNED;
    atomic_uint_fast64_t head_seq CACHE_ALIGNED;
    atomic_uint_fast64_t tail_seq CACHE_ALIGNED;
    atomic_uint_fast64_t nodes[] CACHE_ALIGNED;
} LFRing;

__always_inline uint32_t LFRing_push(LFRing *ring, uint32_t id)
{
    uint64_t busy_loop = 0;
    uint64_t expired_node = 0;
    uint64_t cur_head;
    uint64_t cur_node, expected_node, new_node;
    uint64_t mask = ring->node_count_mask;
    atomic_uint_fast64_t *atom_cur_node;
    
    for (;;) {
        cur_head = ring->head_seq;
        atom_cur_node = &ring->nodes[cur_head & mask];
        cur_node = *atom_cur_node;
        expected_node = (cur_head << 32) | LFRING_INVALID_ID;
        if (cur_node == expected_node) {
            new_node = (cur_head << 32) | id;
            if (atomic_compare_exchange_weak(atom_cur_node, &expected_node, new_node))
                break;
            expired_node = 0;
            busy_loop = 0;
            continue;
        }
        if (expired_node == cur_node) {
            ++busy_loop;
            if (busy_loop >= LFRING_MAX_BUSYLOOP_COUNT) {
                atomic_compare_exchange_weak(&ring->head_seq, &cur_head, cur_head + 1);
                expired_node = 0;
                busy_loop = 0;
            }
        } else {
            expired_node = cur_node;
        }
    }

    atomic_compare_exchange_weak(&ring->head_seq, &cur_head, cur_head + 1);
    return cur_head;
}

__always_inline uint32_t LFRing_pop(LFRing *ring, uint64_t *out_seq)
{
    uint32_t id;
    uint64_t busy_loop = 0;
    uint64_t expired_node = 0;
    uint64_t cur_tail;
    uint64_t cur_node, expected_node, new_node;
    uint64_t mask = ring->node_count_mask;
    atomic_uint_fast64_t *atom_cur_node;

    for (;;) {
        if (ring->head_seq == ring->tail_seq)
            return LFRING_INVALID_ID;
        cur_tail = ring->tail_seq;
        atom_cur_node = &ring->nodes[cur_tail & mask];
        cur_node = *atom_cur_node;
        if ((cur_tail & LFRING_MASK_LOW32) == (cur_node >> 32) && (cur_node & LFRING_MASK_LOW32) != LFRING_INVALID_ID) {
            new_node = ((cur_tail + ring->node_count) << 32) | LFRING_INVALID_ID;
            if (atomic_compare_exchange_weak(atom_cur_node, &cur_node, new_node))
                break;
            expired_node = 0;
            busy_loop = 0;
            continue;
        }
        if (expired_node == cur_node) {
            ++busy_loop;
            if (busy_loop >= LFRING_MAX_BUSYLOOP_COUNT) {
                atomic_compare_exchange_weak(&ring->tail_seq, &cur_tail, cur_tail + 1);
                expired_node = 0;
                busy_loop = 0;
            }
        } else {
            expired_node = cur_node;
        }
    }

    atomic_compare_exchange_weak(&ring->tail_seq, &cur_tail, cur_tail + 1);

    if (out_seq) {
        *out_seq = cur_tail;
    }

    return id;
}

#ifdef __cplusplus
}
#endif

#endif
