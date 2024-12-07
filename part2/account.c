#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
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

    } else if(tType == 'T') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        info->target_acc->balance += amount;

    } else if(tType == 'W') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;

    } else if(tType == 'C') {
        numCheck++;
        if(numCheck % 500 == 0) {
            printf("Numcheck: %d\n", numCheck);
            pthread_mutex_lock(&pipe_lock); // Lock for thread-safe pipe access
            time_t current_time = time(NULL);
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                    "Worker checked balance of account %s. Balance is $%.2f. Check occurred at %s",
                    info->acc->account_number, info->acc->balance, ctime(&current_time));
            write(pipe_fd[1], buffer, strlen(buffer)); // Use write() for pipes
            pthread_mutex_unlock(&pipe_lock); // Unlock after writing
        }
    }

    // Unlock account after processing transaction
    pthread_mutex_unlock(&info->acc->ac_lock);
    return NULL;
}


void* update_balance(void* arg) {
    // Iterate through the array of accounts
    for (int i = 0; i < numAcc; i++) {
        // Update the balance using the transaction tracker
        pthread_mutex_lock(&accounts[i].ac_lock);                 
        accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
        pthread_mutex_unlock(&accounts[i].ac_lock);
    }

    // Write final balances to pipe
    pthread_mutex_lock(&pipe_lock); // Lock pipe access
    for (int i = 0; i < numAcc; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Final balance of account %s: %.2f\n",
                accounts[i].account_number, accounts[i].balance);
        write(pipe_fd[1], buffer, strlen(buffer)); // Use write() for pipes
    }
    pthread_mutex_unlock(&pipe_lock); // Unlock pipe access
    
    return NULL;
}

