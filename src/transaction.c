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

    pthread_mutex_lock(&tick_lock);
	tx->actual_start = global_tick;
	printf("Tick %d:\n",global_tick);
	pthread_mutex_unlock(&tick_lock);



    tx->status = TX_RUNNING;

    for (int i = 0;i < tx->num_ops;i++) {

        Operation *op = &tx->ops[i];
        load_account(&buffer_pool,op->account_id);

        switch (op->type) {
            case OP_DEPOSIT:
				printf("T%d started: DEPOSIT account %d amount PHP %d.%02d\n",
           			tx->tx_id,
           			op->account_id,
           			op->amount_centavos / 100,
           			op->amount_centavos % 100);

                deposit(op->account_id,op->amount_centavos);
				printf("T%d completed: DEPOSIT successful\n", tx->tx_id);
                break;

            case OP_WITHDRAW:
				printf("T%d started: WITHDRAW account %d amount PHP %d.%02d\n",
           			tx->tx_id,
           			op->account_id,
           			op->amount_centavos / 100,
           			op->amount_centavos % 100);
                if (!withdraw(op->account_id,op->amount_centavos)) {
					printf("T%d aborted: insufficient funds\n",tx->tx_id);
                    tx->status =
                        TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);
					unload_account(&buffer_pool, op->account_id);
                    return NULL;
                }
				printf("T%d completed: WITHDRAW successful\n",tx->tx_id);
                break;

            case OP_TRANSFER:

				    printf("T%d started: TRANSFER from %d to %d amount PHP %d.%02d\n",
           				tx->tx_id,
           				op->account_id,
           				op->target_account,
           				op->amount_centavos / 100,
           				op->amount_centavos % 100);
                if (!transfer(tx->tx_id, op->account_id, op->target_account, op->amount_centavos)){
                    tx->status =
                        TX_ABORTED;
                    pthread_mutex_lock(&metrics.lock);
                    metrics.aborted++;
                    pthread_mutex_unlock(&metrics.lock);
					unload_account(&buffer_pool, op->account_id);
                    return NULL;
                }
				printf("T%d completed: TRANSFER successful\n",tx->tx_id);
                break;

            case OP_BALANCE:
                printf("T%d: Account %d balance = PHP %d.%02d\n",
       				tx->tx_id,
       				op->account_id,
       				get_balance(op->account_id) / 100,
       				get_balance(op->account_id) % 100);

                break;
        }

        unload_account(&buffer_pool, op->account_id);
    }

    pthread_mutex_lock(&tick_lock);
	tx->actual_end = global_tick;
	pthread_mutex_unlock(&tick_lock);

    tx->status = TX_COMMITTED;
    pthread_mutex_lock(&metrics.lock);
    metrics.committed++;
    pthread_mutex_unlock(&metrics.lock);

    return NULL;
}