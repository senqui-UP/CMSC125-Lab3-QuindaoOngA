#include "buffer_pool.h"

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
void load_account(BufferPool *pool, int account_id){

    // Block if buffer pool is full
    sem_wait(&pool->empty_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {

        // Find first available slot
        if (!pool->slots[i].in_use) {

            pool->slots[i].account_id = account_id;

            // Store pointer to actual bank account data
            pool->slots[i].data = &bank.accounts[account_id];

            pool->slots[i].in_use = true;

            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    // Notify that a slot is now occupied
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
void unload_account(BufferPool *pool, int account_id)
{
    // Block if no loaded accounts exist
    sem_wait(&pool->full_slots);

    pthread_mutex_lock(&pool->pool_lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {

        // Find matching loaded account
        if (pool->slots[i].in_use &&
            pool->slots[i].account_id == account_id) {

            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;

            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    // Notify that a free slot is available
    sem_post(&pool->empty_slots);
}