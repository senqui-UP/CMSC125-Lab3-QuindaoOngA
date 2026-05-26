#include <stdio.h>

#include "bank.h"
#include "lock_mgr.h"
#include "timer.h"

Bank bank;

int get_balance(int account_id){
    Account *acc = &bank.accounts[account_id];
    pthread_rwlock_rdlock(&acc->lock);
    int balance = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);
    return balance;
}

void deposit(int account_id, int amount){
    Account *acc = &bank.accounts[account_id];
    pthread_rwlock_wrlock(&acc->lock);
    acc->balance_centavos += amount;
    pthread_rwlock_unlock(&acc->lock);
}

bool withdraw(int account_id,int amount){
    Account *acc = &bank.accounts[account_id];
    pthread_rwlock_wrlock(&acc->lock);
    if (acc->balance_centavos < amount) {
        pthread_rwlock_unlock(&acc->lock);
        return false;
    }
    acc->balance_centavos -= amount;
    pthread_rwlock_unlock(&acc->lock);
    return true;
}

bool transfer(int tx_id, int from_id, int to_id, int amount) {
    Account *from = &bank.accounts[from_id];
    Account *to   = &bank.accounts[to_id];

    pthread_rwlock_wrlock(&from->lock);
    from->lock_owner = tx_id;

    pthread_mutex_lock(&print_lock);
    printf("T%d acquired lock on account %d\n", tx_id, from_id);
    pthread_mutex_unlock(&print_lock);

    if (pthread_rwlock_trywrlock(&to->lock) != 0) {
        record_wait(tx_id, to_id, to->lock_owner);

        pthread_mutex_lock(&print_lock);
        printf("[DEADLOCK PREVENTED] Lock ordering: T%d waiting for account %d\n",
               tx_id, to_id);
        pthread_mutex_unlock(&print_lock);

        if (detect_deadlock()) {
            pthread_mutex_lock(&print_lock);
            printf("[DEADLOCK DETECTED] Transaction %d aborted\n", tx_id);
            pthread_mutex_unlock(&print_lock);
            pthread_rwlock_unlock(&from->lock);
            clear_wait(tx_id);
            return false;
        }
        pthread_rwlock_wrlock(&to->lock);
    }

    to->lock_owner = tx_id;

    pthread_mutex_lock(&print_lock);
    printf("T%d acquired lock on account %d\n", tx_id, to_id);
    pthread_mutex_unlock(&print_lock);

    if (from->balance_centavos < amount) {
        pthread_rwlock_unlock(&to->lock);
        pthread_rwlock_unlock(&from->lock);
        clear_wait(tx_id);
        return false;
    }

    from->balance_centavos -= amount;
    to->balance_centavos   += amount;
    pthread_rwlock_unlock(&to->lock);
    pthread_rwlock_unlock(&from->lock);
    clear_wait(tx_id);
    return true;
}