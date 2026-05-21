#ifndef CMSC125_LAB3_QUINDAOONGA_BUFFER_POOL_H
#define CMSC125_LAB3_QUINDAOONGA_BUFFER_POOL_H

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "bank.h"

#define BUFFER_POOL_SIZE 5

typedef struct {
    int account_id;
    Account *data;
    bool in_use;
} BufferSlot;

typedef struct {
    BufferSlot slots[BUFFER_POOL_SIZE];

    sem_t empty_slots;
    sem_t full_slots;

    pthread_mutex_t pool_lock;
} BufferPool;

/* Global buffer pool */
extern BufferPool buffer_pool;

void init_buffer_pool(BufferPool *pool);
void load_account(BufferPool *pool, int account_id);
void unload_account(BufferPool *pool, int account_id);

#endif //CMSC125_LAB3_QUINDAOONGA_BUFFER_POOL_H