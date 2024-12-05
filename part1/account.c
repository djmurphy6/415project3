#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"

int updateCounter = 0;

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

    } else if(tType == 'T') {
        amount = info.amount;
        acc.transaction_tracker -= amount;
        info.target_acc.transaction_tracker += amount;

    } else if(tType == 'W') {
        amount = info.amount;
        acc.transaction_tracker -= amount;

    } else if(tType == 'C') {
        /**
        FILE* out_file = fopen(acc.out_file, "a");
        if (out_file == NULL) {
            perror("Error opening file");
            return -1;
        }
        fprintf(out_file, "Current Savings Balance %.2f\n", (acc.balance + acc.transaction_tracker));
        fclose(out_file);
        */
    }
    return 0;
}


int update_balance() {
    // Iterate through the array of accounts
    for (int i = 0; i < numAcc; i++) {
        // Update the balance using the transaction tracker
        accounts[i].balance += accounts[i].transaction_tracker;

        // Reset the transaction tracker
        accounts[i].transaction_tracker = 0;

        // Optionally log the updated balance to the account's output file
        FILE* out_file = fopen(accounts[i].out_file, "a");
        if (out_file == NULL) {
            perror("Error opening account file");
            return -1; // Exit with an error code if file open fails
        }
        fprintf(out_file, "Updated Savings Balance: %.2f\n", accounts[i].balance);
        fclose(out_file);
    }

    updateCounter++; // Increment the balance update counter
    return 0; // Return 0 on successful update
}
