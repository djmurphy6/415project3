#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"
#include <sys/mman.h>  // mmap
#include <unistd.h>    // for ftruncate

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
    // Synchronize threads using the barrier
    transaction* info = (transaction*)arg;
    
    char tType = info->tType;
    double amount = 0;

    // Lock account before processing transaction
    pthread_mutex_lock(&info->acc->ac_lock);

    if (tType == 'D') {
        amount = info->amount;
        info->acc->transaction_tracker += amount;
        info->acc->balance += amount;
    } else if (tType == 'T') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        info->target_acc->balance += amount;
    } else if (tType == 'W') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
    } else if (tType == 'C') {
        // No change in balance for balance check
    }

    // Unlock account after processing transaction
    pthread_mutex_unlock(&info->acc->ac_lock);
    return NULL;
}

void* update_balance(void* arg) {
    pthread_mutex_lock(&counter_lock);

    // Shared memory segment setup for Duck and Puddles Bank synchronization
    account* accounts_puddles = mmap(NULL, numAcc * sizeof(account), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (accounts_puddles == MAP_FAILED) {
        perror("Error creating shared memory");
        return NULL;
    }

    // Duplicate Duck Bank accounts to Puddles Bank
    for (int i = 0; i < numAcc; i++) {
        accounts_puddles[i].balance = accounts[i].balance * 0.2;
        accounts_puddles[i].reward_rate = 0.02;  // 2% reward rate
    }

    while (1) {
        while (counter < TRANSACTIONS_THRESHOLD && !(done && transactions_processed >= 90000)) {
            pthread_cond_wait(&bank_cond, &counter_lock);
        }

        transactions_processed += counter;
        counter = 0;

        for (int i = 0; i < numAcc; i++) {
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
            accounts[i].transaction_tracker = 0;

            // Apply the 2% reward for Puddles Bank savings account
            accounts_puddles[i].balance += (accounts_puddles[i].balance * accounts_puddles[i].reward_rate);
            char filename[20];
            sprintf(filename, "Output/account_%d.txt", i);
            FILE* file = fopen(filename, "a");
            if (file != NULL) {
                fprintf(file, "Current Balance (Puddles): %.2f\n", accounts_puddles[i].balance);
                fclose(file);
            } else {
                perror("Error opening file");
            }

            pthread_mutex_unlock(&accounts[i].ac_lock);
        }

        if (transactions_processed >= 90000) {
            break;
        }
    }

    return NULL;
}