#include <stdio.h>
#include "transaction.h"
#include "bank.h"
#include "timer.h"
#include "metrics.h"
#include "buffer_pool.h"

void *execute_transaction(void *arg) {
    Transaction *tx = (Transaction *)arg;

    // Wait for this transaction's start tick
    wait_until_tick(tx->start_tick);

    pthread_mutex_lock(&tick_lock);
    tx->actual_start = global_tick;
    pthread_mutex_unlock(&tick_lock);

    tx->status = TX_RUNNING;

    // Load once at transaction start
    load_account(&buffer_pool, tx->ops[0].account_id);

    for (int i = 0; i < tx->num_ops; i++) {
        Operation *op = &tx->ops[i];

        int current_tick;
        pthread_mutex_lock(&tick_lock);
        current_tick = global_tick;
        pthread_mutex_unlock(&tick_lock);

        switch (op->type) {
            case OP_DEPOSIT:
                pthread_mutex_lock(&print_lock);
                printf("T%d started: DEPOSIT account %d amount PHP %d.%02d\n",
                       tx->tx_id, op->account_id,
                       op->amount_centavos / 100,
                       op->amount_centavos % 100);
                pthread_mutex_unlock(&print_lock);

                // Operation takes one tick
                wait_until_tick(current_tick + 1);
                deposit(op->account_id, op->amount_centavos);

                pthread_mutex_lock(&print_lock);
                printf("T%d completed: DEPOSIT successful\n", tx->tx_id);
                pthread_mutex_unlock(&print_lock);
                break;

            case OP_WITHDRAW:
                pthread_mutex_lock(&print_lock);
                printf("T%d started: WITHDRAW account %d amount PHP %d.%02d\n",
                       tx->tx_id, op->account_id,
                       op->amount_centavos / 100,
                       op->amount_centavos % 100);
                pthread_mutex_unlock(&print_lock);

                wait_until_tick(current_tick + 1);

                if (!withdraw(op->account_id, op->amount_centavos)) {
                    pthread_mutex_lock(&print_lock);
                    printf("[TX ABORTED] Transaction %d failed: Insufficient funds\n",
                           tx->tx_id);
                    pthread_mutex_unlock(&print_lock);

                    tx->status = TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);

                    // Unload on abort
                    unload_account(&buffer_pool, tx->ops[0].account_id);
                    return NULL;
                }

                pthread_mutex_lock(&print_lock);
                printf("T%d completed: WITHDRAW successful\n", tx->tx_id);
                pthread_mutex_unlock(&print_lock);
                break;

            case OP_TRANSFER:
                pthread_mutex_lock(&print_lock);
                printf("T%d started: TRANSFER from %d to %d amount PHP %d.%02d\n",
                       tx->tx_id, op->account_id, op->target_account,
                       op->amount_centavos / 100,
                       op->amount_centavos % 100);
                pthread_mutex_unlock(&print_lock);

                // Lock acquisition on next tick
                wait_until_tick(current_tick + 1);

                if (!transfer(tx->tx_id, op->account_id,
                              op->target_account, op->amount_centavos)) {
                    tx->status = TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);

                    unload_account(&buffer_pool, tx->ops[0].account_id);
                    return NULL;
                }

                // Completion on tick after lock acquisition
                wait_until_tick(current_tick + 2);

                pthread_mutex_lock(&print_lock);
                printf("T%d completed: TRANSFER successful\n", tx->tx_id);
                pthread_mutex_unlock(&print_lock);
                break;

            case OP_BALANCE:
                pthread_mutex_lock(&print_lock);
                printf("T%d started: BALANCE account %d\n",
                       tx->tx_id, op->account_id);
                pthread_mutex_unlock(&print_lock);

                wait_until_tick(current_tick + 1);

                pthread_mutex_lock(&print_lock);
                printf("T%d: Account %d balance = PHP %d.%02d\n",
                       tx->tx_id, op->account_id,
                       get_balance(op->account_id) / 100,
                       get_balance(op->account_id) % 100);
                pthread_mutex_unlock(&print_lock);
                break;
        }
    }

    // Unload once at transaction commit
    unload_account(&buffer_pool, tx->ops[0].account_id);

    pthread_mutex_lock(&tick_lock);
    tx->actual_end = global_tick;
    pthread_mutex_unlock(&tick_lock);

    tx->status = TX_COMMITTED;
    pthread_mutex_lock(&metrics.lock);
    metrics.committed++;
    pthread_mutex_unlock(&metrics.lock);

    return NULL;
}