#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"

// Helper function to find an account by account number
account* find_account(account* accounts, int numAcc, const char* account_number) {
    for (int i = 0; i < numAcc; i++) {
        if (strcmp(accounts[i].account_number, account_number) == 0) {
            return &accounts[i];
        }
    }
    return NULL; // Account not found
}

void* process_transaction(void* arg) {
    // Cast the argument to a transaction struct
    transaction* info = (transaction*)arg;
    
    char tType = info->tType;
    double amount = 0;

    // Lock account before processing transaction
    pthread_mutex_lock(&info->acc->ac_lock);

    if(tType == 'D') {
        amount = info->amount;
        info->acc->transaction_tracker += amount;
        info->acc->balance += amount;
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'T') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        info->target_acc->balance += amount;
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'W') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'C') {
    }

    // Unlock account after processing transaction
    pthread_mutex_unlock(&info->acc->ac_lock);
    return NULL;
}


void* update_balance(void* arg) {
    printf("Bank thread started\n");
    while (1) {

        // Wait until enough transactions have been processed or all transactions are done
        while (counter < TRANSACTIONS_THRESHOLD && !(done && transactions_processed >= total_transactions)) {
            pthread_cond_wait(&bank_cond, &counter_lock);
        }

        // Exit the loop if processing is complete
        if (done && transactions_processed >= total_transactions) {
            pthread_mutex_unlock(&counter_lock);
            break;
        }

        // Reset counter and allow worker threads to continue
        transactions_processed += counter;
        counter = 0;

        // Perform the balance update
        for (int i = 0; i < numAcc; i++) {
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
            accounts[i].transaction_tracker = 0;
            pthread_mutex_unlock(&accounts[i].ac_lock);
        }

        // Synchronize threads using the barrier
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

