#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "queue.h"

LFQueue *queue;
int key = -1;
int count = -1;
int size = -1;
int nthread = 1;
bool overite = false;
bool pause_before_print = false;

void create_mq()
{
        if (LFQueue_create(key, size, count, overite) != 0) {
                perror("[FAILURE] Error occured in LFQueue_create");
                exit(-1);
        }
        printf("[SUCESS] Create LFQueue\n");
}

void destroy_mq()
{
        if (LFQueue_destroy(key) != 0) {
                perror("[FAILURE] Error occured in LFQueue_destroy");
                exit(-1);
        }
}

void *produce_task(void *arg)
{
        LFNode *node = (LFNode *)malloc(queue->header->node_total_size);
        node->size = queue->header->node_data_size;
        for (;;)
                LFQueue_push(queue, node);
}

void *consume_task(void *arg)
{
        LFNode *node = (LFNode *)malloc(queue->header->node_total_size);
        for (;;)
                LFQueue_pop(queue, node);
}

void produce()
{
        int i;

        queue = LFQueue_open(key);
        if (!queue) {
                printf("[FAILURE] Error occured in LFQueue_open\n");
                exit(-1);
        }

        for (i = 0; i < nthread - 1; ++i) {
                pthread_t th;
                pthread_create(&th, NULL, produce_task, NULL);
        }

        produce_task(NULL);
}

void consume()
{
        int i;

        queue = LFQueue_open(key);
        if (!queue) {
                printf("[FAILURE] Error occured in LFQueue_open\n");
                exit(-1);
        }

        for (i = 0; i < nthread - 1; ++i) {
                pthread_t th;
                pthread_create(&th, NULL, consume_task, NULL);
        }

        consume_task(NULL);
}

void print_mq()
{
        queue = LFQueue_open(key);
        if (!queue) {
                printf("[FAILURE] Error occured in LFQueue_open\n");
                exit(-1);
        }

        if (pause_before_print) {
                LFQueue_pause(queue);
                usleep(100 * 1000);
                LFQueue_print(queue); 
                LFQueue_resume(queue);
        } else {
                LFQueue_print(queue);
        }
        
}

void monitor()
{
        uint64_t last;
        LFRing *ring;

        queue = LFQueue_open(key);
        if (!queue) {
                printf("[FAILURE] Error occured in LFQueue_open\n");
                exit(-1);
        }

        ring = queue->node_ring;
        last = ring->head_seq;
        for (;;) {
                printf("speed: %lu\ttotal: %lu\n", ring->head_seq - last, ring->head_seq);
                last = ring->head_seq;
                sleep(1);
        }
}

void reset()
{
        queue = LFQueue_open(key);
        if (!queue) {
                printf("[FAILURE] Error occured in LFQueue_open\n");
                exit(-1);
        }

        LFQueue_pause(queue);
        usleep(1000 * 100);

        LFQueue_reset(queue);

        LFQueue_resume(queue);
}

int main(int argc, char *argv[])
{
        int opt;

        const char *usage = "Usage: [-c:hk:ops:t:]\n"
                "\t./cli create [-k key] [-s size] [-c count] [-o]\n"
                "\t./cli destroy [-k key]\n"
                "\t./cli reset [-k key]"
                "\t./cli status [-k key]\n"
                "\t./cli monitor [-k key]\n"
                "\t./cli produce [-k key] [-t nthread]\n"
                "\t./cli consume [-k key] [-t nthread]\n";

        if (argc <= 2) {
                printf("%s", usage);
                exit(-1);
        }

        while ((opt = getopt(argc, argv, "c:hk:ops:t:")) != -1) {
                switch (opt) {
                case 'c':
                        count = atoi(optarg);
                        break;
                case 'h':
                        printf("%s", usage);
                        exit(0);
                case 'k':
                        key = atoi(optarg);
                        break;
                case 'o':
                        overite = true;
                        break;
                case 'p':
                        pause_before_print = true;
                        break;
                case 's':
                        size = atoi(optarg);
                        break;
                case 't':
                        nthread = atoi(optarg);
                default:
                        break;
                }
        }

        if (strcmp(argv[argc - 1], "create") == 0)
                create_mq();
        else if (strcmp(argv[argc - 1], "destroy") == 0)
                destroy_mq();
        else if (strcmp(argv[argc - 1], "produce") == 0)
                produce();
        else if (strcmp(argv[argc - 1], "consume") == 0)
                consume();
        else if (strcmp(argv[argc - 1], "status") == 0)
                print_mq();
        else if (strcmp(argv[argc - 1], "monitor") == 0)
                monitor();
        else if (strcmp(argv[argc - 1], "reset") == 0)
                reset();
}
