#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "bank.h"
#include "transaction.h"
#include "timer.h"
#include "metrics.h"
#include "buffer_pool.h"
#include "lock_mgr.h"
#include "utils.h"

// Global transaction storage
Transaction transactions[MAX_TRANSACTIONS];

int transaction_count = 0;

// Initialize wait-for graph
void initialize_wait_graph()
{
    for (int i = 0;
         i < MAX_TRANSACTIONS;
         i++) {

        wait_graph[i].tx_id = i;

        wait_graph[i].waiting_for_tx = -1;

        wait_graph[i].waiting_for_account = -1;
    }
}

int main(int argc, char *argv[])
{
    char accounts_file[256] =
        "tests/accounts.txt";

    char trace_file[256] =
        "tests/trace.txt";

    int tick_ms = 100;

    pthread_t timer;

    // Parse CLI arguments
    for (int i = 1;
         i < argc;
         i++) {

        if (strncmp(argv[i],
                    "--accounts=",
                    11) == 0) {

            strcpy(accounts_file,
                   argv[i] + 11);
        }

        else if (strncmp(argv[i],
                         "--trace=",
                         8) == 0) {

            strcpy(trace_file,
                   argv[i] + 8);
        }

        else if (strncmp(argv[i],
                         "--tick-ms=",
                         10) == 0) {

            tick_ms =
                atoi(argv[i] + 10);
        }
    }

    // Initialize metrics mutex
    pthread_mutex_init(
        &metrics.lock,
        NULL);

    // Initialize wait-for graph
    initialize_wait_graph();

    // Initialize bank mutex
    pthread_mutex_init(
        &bank.bank_lock,
        NULL);

    // Initialize buffer pool
    init_buffer_pool(
        &buffer_pool);

    // Load accounts
    load_accounts(accounts_file);

    // Load transactions
    load_transactions(trace_file);

    // Initialize metrics
    metrics.total_transactions =
        transaction_count;

    metrics.committed = 0;

    metrics.aborted = 0;

    // Start timer thread
    pthread_create(
        &timer,
        NULL,
        timer_thread,
        &tick_ms);

    printf("=== Banking System Execution Log ===\n");

    printf("Timer thread started (tick interval: %dms)\n\n",
           tick_ms);

    // Start transaction threads
    for (int i = 0;
         i < transaction_count;
         i++) {

        pthread_create(
            &transactions[i].thread,
            NULL,
            execute_transaction,
            &transactions[i]);
    }

    // Wait for all transaction threads
    for (int i = 0;
         i < transaction_count;
         i++) {

        pthread_join(
            transactions[i].thread,
            NULL);
    }

    // Stop timer thread
    simulation_running = false;

    pthread_join(timer, NULL);

    // Print final balances
    printf("\n=== Final Balances ===\n");

    for (int i = 0;
         i < MAX_ACCOUNTS;
         i++) {

        if (bank.accounts[i].balance_centavos
            > 0) {

            printf("Account %d : PHP %d.%02d\n",
                   bank.accounts[i].account_id,
                   bank.accounts[i].balance_centavos / 100,
                   bank.accounts[i].balance_centavos % 100);
        }
    }

    // Print metrics
    print_metrics();

    // Cleanup account locks
    for (int i = 0;
         i < MAX_ACCOUNTS;
         i++) {

        if (bank.accounts[i].balance_centavos
            > 0) {

            pthread_rwlock_destroy(
                &bank.accounts[i].lock);
        }
    }

    // Cleanup synchronization primitives
    pthread_mutex_destroy(
        &metrics.lock);

    pthread_mutex_destroy(
        &bank.bank_lock);

    pthread_mutex_destroy(
        &graph_lock);

    pthread_mutex_destroy(
        &tick_lock);

    pthread_cond_destroy(
        &tick_changed);

    return 0;
}