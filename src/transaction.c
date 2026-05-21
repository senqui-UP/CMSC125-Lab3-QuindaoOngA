#include <stdio.h>

#include "transaction.h"
#include "bank.h"
#include "timer.h"
#include "metrics.h"
#include "buffer_pool.h"

void *execute_transaction(void *arg){

    Transaction *tx =
        (Transaction *)arg;
    wait_until_tick(tx->start_tick);
    tx->actual_start = global_tick;
    tx->status = TX_RUNNING;

    for (int i = 0;i < tx->num_ops;i++) {

        Operation *op = &tx->ops[i];
        load_account(&buffer_pool,op->account_id);

        switch (op->type) {
            case OP_DEPOSIT:
                deposit(op->account_id,op->amount_centavos);
                break;

            case OP_WITHDRAW:
                if (!withdraw(op->account_id,op->amount_centavos)) {
                    tx->status =
                        TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);
                    return NULL;
                }
                break;

            case OP_TRANSFER:
                if (!transfer(tx->tx_id, op->account_id, op->target_account, op->amount_centavos))
                    tx->status =
                        TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);
                    return NULL;
                }
                break;

            case OP_BALANCE:
                printf("Balance = %d\n",
                       get_balance(op->account_id));

                break;
        }

        unload_account(&buffer_pool, op->account_id);
    }

    tx->actual_end = global_tick;
    tx->status = TX_COMMITTED;
    pthread_mutex_lock(&metrics.lock);
    metrics.committed++;
    pthread_mutex_unlock(&metrics.lock);

    return NULL;
}