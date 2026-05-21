#ifndef CMSC125_LAB3_QUINDAOONGA_LOCK_MGR_H
#define CMSC125_LAB3_QUINDAOONGA_LOCK_MGR_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_TRANSACTIONS 100

typedef struct {
    int tx_id;

    int waiting_for_tx;
    int waiting_for_account;

} WaitForEntry;

extern WaitForEntry wait_graph[MAX_TRANSACTIONS];

extern pthread_mutex_t graph_lock;

void record_wait(int tx_id, int account_id, int holder_tx);

void clear_wait(int tx_id);

bool detect_deadlock();
#endif //CMSC125_LAB3_QUINDAOONGA_LOCK_MGR_H