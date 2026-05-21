#ifndef CMSC125_LAB3_QUINDAOONGA_TRANSACTION_H
#define CMSC125_LAB3_QUINDAOONGA_TRANSACTION_H

#include <pthread.h>

#define MAX_OPS 256
#define MAX_TRANSACTIONS 100

typedef enum {
    OP_DEPOSIT,   // Add money to account
    OP_WITHDRAW,  // Remove money from account
    OP_TRANSFER,  // Move money between two accounts
    OP_BALANCE    // Read account balance
} OpType;

typedef struct {
    OpType type;
    int account_id;          // Primary account
    int amount_centavos;     // Amount in centavos
    int target_account;      // For TRANSFER only
} Operation;

typedef enum {
    TX_RUNNING,
    TX_COMMITTED,
    TX_ABORTED
} TxStatus;

typedef struct {

    int tx_id;
    Operation ops[MAX_OPS];
    int num_ops;
    int start_tick;
    pthread_t thread;

    /* Timing */
    int actual_start;
    int actual_end;
    int wait_ticks;

    /* Status */
    TxStatus status;
} Transaction;

/* Global transaction list */
extern Transaction transactions[MAX_TRANSACTIONS];
extern int transaction_count;
void *execute_transaction(void *arg);

#endif //CMSC125_LAB3_QUINDAOONGA_TRANSACTION_H