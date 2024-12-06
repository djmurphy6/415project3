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

int process_transaction(transaction info) {
    
    char tType = info.tType;
    double amount = 0;

    if(tType == 'D') {
        amount = info.amount;
        info.acc -> transaction_tracker += amount;
        info.acc -> balance += amount;

    } else if(tType == 'T') {
        amount = info.amount;
        info.acc->balance -= amount;
        info.acc->transaction_tracker += amount;
        info.target_acc->balance += amount;


    } else if(tType == 'W') {
        amount = info.amount;
        info.acc->balance -= amount;
        info.acc->transaction_tracker += amount;

    } else if(tType == 'C') {
    }
    return 0;
}


void* update_balance(void* arg) {
    // Iterate through the array of accounts
    for (int i = 0; i < numAcc; i++) {
        // Update the balance using the transaction tracker                 
        accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);

        // Reset the transaction tracker
        accounts[i].transaction_tracker = 0;

    }
    return NULL;
}
