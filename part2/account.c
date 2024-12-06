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
    }

    // Unlock account after processing transaction
    pthread_mutex_unlock(&info->acc->ac_lock);
    printf("Transaction processed\n");
    return NULL;
}


void* update_balance(void* arg) {
    // Iterate through the array of accounts
    for (int i = 0; i < numAcc; i++) {
        // Update the balance using the transaction tracker                 
        accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
    }
    return NULL;
}

