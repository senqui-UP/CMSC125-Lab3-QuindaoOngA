#include <stdio.h>
#include "buffer_pool.h"
#include "timer.h"
#include "metrics.h"

// Global buffer pool instance
// Simulates a limited in-memory cache of bank accounts
BufferPool buffer_pool;

/*
 * Initializes the buffer pool
 * empty_slots:
 *     Counts available buffer slots
 *     Starts at BUFFER_POOL_SIZE because all slots are initially free
 * full_slots:
 *     Counts occupied buffer slots
 *     Starts at 0 because nothing is loaded yet
 * pool_lock:
 *     Protects concurrent access to buffer pool metadata
 */
void init_buffer_pool(BufferPool *pool){

    sem_init(&pool->empty_slots, 0, BUFFER_POOL_SIZE);
    sem_init(&pool->full_slots, 0, 0);

    pthread_mutex_init(&pool->pool_lock, NULL);

    // Mark all buffer slots as unused
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        pool->slots[i].in_use = false;
        pool->slots[i].account_id = -1;
        pool->slots[i].data = NULL;
    }
}

/*
 * LOADS account into the buffer pool
 * 1. Wait for an empty slot to become available.
 * 2. Lock the pool for safe concurrent modification.
 * 3. Find a free slot and assign account data.
 * 4. Unlock the pool.
 * 5. Signal that a slot is now occupied.
 */
void load_account(BufferPool *pool, int account_id) {
    // Check if we would block (pool is full)
    int val;
    sem_getvalue(&pool->empty_slots, &val);

    if (val == 0) {
        pthread_mutex_lock(&print_lock);
        printf("[BUFFER POOL FULL] Account %d waiting for free slot\n",
               account_id);
        pthread_mutex_unlock(&print_lock);

        pthread_mutex_lock(&metrics.lock);
        metrics.buffer_waits++;
        pthread_mutex_unlock(&metrics.lock);
    }

    // Block here if pool is full
    sem_wait(&pool->empty_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (!pool->slots[i].in_use) {
            pool->slots[i].account_id = account_id;
            pool->slots[i].data = &bank.accounts[account_id];
            pool->slots[i].in_use = true;

            pthread_mutex_lock(&print_lock);
            printf("[BUFFER POOL] Account %d loaded into slot %d\n",
                   account_id, i);
            pthread_mutex_unlock(&print_lock);

            break;
        }
    }

    pthread_mutex_lock(&metrics.lock);
    metrics.buffer_loads++;
    pthread_mutex_unlock(&metrics.lock);

    pthread_mutex_unlock(&pool->pool_lock);
    sem_post(&pool->full_slots);
}

/*
 * REMOVES account from the buffer pool
 * 1. Wait until at least one slot is occupied
 * 2. Lock the pool for safe modification
 * 3. Find the matching account slot
 * 4. Mark slot as free
 * 5. Unlock the pool
 * 6. Signal that an empty slot is available
 */

void unload_account(BufferPool *pool, int account_id) {
    sem_wait(&pool->full_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (pool->slots[i].in_use &&
            pool->slots[i].account_id == account_id) {

            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;

            pthread_mutex_lock(&print_lock);
            printf("[BUFFER POOL] Account %d unloaded from slot %d\n",
                   account_id, i);
            pthread_mutex_unlock(&print_lock);

            break;
        }
    }

    pthread_mutex_lock(&metrics.lock);
    metrics.buffer_unloads++;
    pthread_mutex_unlock(&metrics.lock);

    pthread_mutex_unlock(&pool->pool_lock);
    sem_post(&pool->empty_slots);
}