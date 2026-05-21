#ifndef CMSC125_LAB3_QUINDAOONGA_BANK_H
#define CMSC125_LAB3_QUINDAOONGA_BANK_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 100

typedef struct {
    int account_id;          // Account number
    int balance_centavos;    // Balance in centavos
    pthread_rwlock_t lock;   // Per-account lock
    int lock_owner;          // For deadlock detection

} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
    pthread_mutex_t bank_lock; // Protects bank metadata
} Bank;

/* Global bank instance */
extern Bank bank;

/* Bank operations */
int get_balance(int account_id);
void deposit(int account_id,int amount);
bool withdraw(int account_id, int amount);
bool transfer(int tx_id, int from_id, int to_id, int amount);

#endif //CMSC125_LAB3_QUINDAOONGA_BANK_H