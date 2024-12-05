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
    account acc = info.acc;
    char tType = info.tType;
    double amount = 0;

    if(tType == 'D') {
        amount = info.amount;
        acc.transaction_tracker += amount;
        //fprintf(acc.out_file, "Current Savings Balance: %.2f\n", (acc.balance + acc.transaction_tracker));

    } else if(tType == 'T') {
        amount = info.amount;
        acc.transaction_tracker -= amount;
        info.target_acc.transaction_tracker += amount;
        //fprintf(acc.out_file, "Current Savings Balance: %.2f\n", (acc.balance + acc.transaction_tracker));
        //fprintf(info.target_acc.out_file, "Current Savings Balance: %.2f\n", (info.target_acc.balance + info.target_acc.transaction_tracker));

    } else if(tType == 'W') {
        amount = info.amount;
        acc.transaction_tracker -= amount;
        //fprintf(acc.out_file, "Current Savings Balance: %.2f\n", (acc.balance + acc.transaction_tracker));

    } else if(tType == 'C') {
        //fprintf(acc.out_file, "Current Savings Balance: %.2f\n", (acc.balance + acc.transaction_tracker));
    }
    
    return 0;
}

void* update_balance(void* arg) {
    return NULL;
}
