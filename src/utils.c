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

    char line[256];

    transaction_count = 0;

    while (fgets(line,
                 sizeof(line),
                 fp)) {

        // Skip comments
        if (line[0] == '#') {

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

            Transaction *tx =
                &transactions[transaction_count];

            tx->tx_id =
                transaction_count;

            tx->start_tick =
                start_tick;

            tx->num_ops = 1;

            tx->ops[0].type =
                OP_TRANSFER;

            tx->ops[0].account_id =
                account_id;

            tx->ops[0].target_account =
                target_account;

            tx->ops[0].amount_centavos =
                amount;

            transaction_count++;

            continue;
        }

        // DEPOSIT
        if (sscanf(line,
                   "%s %d DEPOSIT %d %d",
                   tx_name,
                   &start_tick,
                   &account_id,
                   &amount) == 4) {

            Transaction *tx =
                &transactions[transaction_count];

            tx->tx_id =
                transaction_count;

            tx->start_tick =
                start_tick;

            tx->num_ops = 1;

            tx->ops[0].type =
                OP_DEPOSIT;

            tx->ops[0].account_id =
                account_id;

            tx->ops[0].amount_centavos =
                amount;

            transaction_count++;

            continue;
        }

        // WITHDRAW
        if (sscanf(line,
                   "%s %d WITHDRAW %d %d",
                   tx_name,
                   &start_tick,
                   &account_id,
                   &amount) == 4) {

            Transaction *tx =
                &transactions[transaction_count];

            tx->tx_id =
                transaction_count;

            tx->start_tick =
                start_tick;

            tx->num_ops = 1;

            tx->ops[0].type =
                OP_WITHDRAW;

            tx->ops[0].account_id =
                account_id;

            tx->ops[0].amount_centavos =
                amount;

            transaction_count++;

            continue;
        }

        // BALANCE
        if (sscanf(line,
                   "%s %d BALANCE %d",
                   tx_name,
                   &start_tick,
                   &account_id) == 3) {

            Transaction *tx =
                &transactions[transaction_count];

            tx->tx_id =
                transaction_count;

            tx->start_tick =
                start_tick;

            tx->num_ops = 1;

            tx->ops[0].type =
                OP_BALANCE;

            tx->ops[0].account_id =
                account_id;

            transaction_count++;
        }
    }

    fclose(fp);
}