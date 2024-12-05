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
        FILE* out_file = fopen(info.acc->out_file, "a");
        if (out_file == NULL) {
            perror("Error opening file");
            return -1;
        }
        fprintf(out_file, "Current Savings Balance %.2f\n", (info.acc->balance + info.acc->transaction_tracker));
        fclose(out_file);
    }
    return 0;
}


void* update_balance(void* arg) {
    // Iterate through the array of accounts
    for (int i = 0; i < numAcc; i++) {
        // Update the balance using the transaction tracker                 
        accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
        // Print the balance before applying the reward rate
        //printf("Balance before reward for account %s: %.2f\n", accounts[i].account_number, accounts[i].balance);
        // Apply the reward rate to the balance
        //accounts[i].balance += accounts[i].balance * accounts[i].reward_rate;
        // Print balance after reward
        //printf("Balance after reward for account %s: %.2f\n", accounts[i].account_number, accounts[i].balance);

        // Reset the transaction tracker
        accounts[i].transaction_tracker = 0;

        // Optionally log the updated balance to the account's output file
        FILE* out_file = fopen(accounts[i].out_file, "a");
        if (out_file == NULL) {
            perror("Error opening account file");
        }
        fprintf(out_file, "Updated Savings Balance: %.2f\n", accounts[i].balance);
        fclose(out_file);
    }
    return NULL;
}
