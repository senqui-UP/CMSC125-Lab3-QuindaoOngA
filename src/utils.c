#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "bank.h"
#include "transaction.h"
#include "utils.h"

// External transaction storage
extern Transaction transactions[MAX_TRANSACTIONS];
extern int transaction_count;

// Global bank instance
extern Bank bank;

// Internal helper: find an existing transaction slot by name, or allocate a new one                                       */
static char tx_names[MAX_TRANSACTIONS][16];
 
static Transaction *find_or_create_tx(const char *tx_name, int start_tick)
{
    // Search existing transactions for a matching name
    for (int i = 0; i < transaction_count; i++) {
        if (strcmp(tx_names[i], tx_name) == 0) {
            return &transactions[i];
        }
    }
 
    // Not found — allocate a new transaction slot
    Transaction *tx  = &transactions[transaction_count];
    tx->tx_id        = transaction_count;
    tx->start_tick   = start_tick;      // first line for this tx sets the start tick
    tx->num_ops      = 0;
    tx->status       = TX_RUNNING;
    tx->actual_start = 0;
    tx->actual_end   = 0;
    tx->wait_ticks   = 0;
 
    strncpy(tx_names[transaction_count], tx_name, 15);
    tx_names[transaction_count][15] = '\0';
 
    transaction_count++;
    return tx;
}

// Load initial account balances from file
void load_accounts(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {

        perror("Failed to open accounts file");

        exit(EXIT_FAILURE);
    }

    char line[256];

    bank.num_accounts = 0;

    while (fgets(line,
                 sizeof(line),
                 fp)) {

        // Skip comments
        if (line[0] == '#') {

            continue;
        }

        int account_id;

        int balance;

        if (sscanf(line,
                   "%d %d",
                   &account_id,
                   &balance) == 2) {

            bank.accounts[account_id].account_id =
                account_id;

            bank.accounts[account_id].balance_centavos =
                balance;

            bank.accounts[account_id].lock_owner =
                -1;

            pthread_rwlock_init(
                &bank.accounts[account_id].lock,
                NULL);

            bank.num_accounts++;
        }
    }

    fclose(fp);
}

// Load transactions from trace file
void load_transactions(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {

        perror("Failed to open trace file");

        exit(EXIT_FAILURE);
    }

    // Reset states
    transaction_count = 0;
    memset(tx_names, 0, sizeof(tx_names));

    char line[256];


    while (fgets(line,
                 sizeof(line),
                 fp)) {

        // Skip comments and blank lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {

            continue;
        }

        char tx_name[16];

        int start_tick;

        int account_id;

        int amount;

        int target_account;

        // TRANSFER
        if (sscanf(line,
                   "%s %d TRANSFER %d %d %d",
                   tx_name,
                   &start_tick,
                   &account_id,
                   &target_account,
                   &amount) == 5) {

            Transaction *tx = find_or_create_tx(tx_name, start_tick);

            Operation *op       = &tx->ops[tx->num_ops++];
            op->type            = OP_TRANSFER;
            op->account_id      = account_id;
            op->target_account  = target_account;
            op->amount_centavos = amount;

            continue;
        }

        // DEPOSIT
        if (sscanf(line,
                   "%s %d DEPOSIT %d %d",
                   tx_name,
                   &start_tick,
                   &account_id,
                   &amount) == 4) {

            Transaction *tx = find_or_create_tx(tx_name, start_tick);

            Operation *op       = &tx->ops[tx->num_ops++];
            op->type            = OP_DEPOSIT;
            op->account_id      = account_id;
            op->amount_centavos = amount;

            continue;
        }

        // WITHDRAW
        if (sscanf(line,
                   "%s %d WITHDRAW %d %d",
                   tx_name,
                   &start_tick,
                   &account_id,
                   &amount) == 4) {

            Transaction *tx = find_or_create_tx(tx_name, start_tick);

            Operation *op       = &tx->ops[tx->num_ops++];
            op->type            = OP_WITHDRAW;
            op->account_id      = account_id;
            op->amount_centavos = amount;

            continue;
        }

        // BALANCE
        if (sscanf(line,
                   "%s %d BALANCE %d",
                   tx_name,
                   &start_tick,
                   &account_id) == 3) {

            Transaction *tx = find_or_create_tx(tx_name, start_tick);

            Operation *op       = &tx->ops[tx->num_ops++];
            op->type            = OP_BALANCE;
            op->account_id      = account_id;

            continue;
        }

    }

    fclose(fp);
}