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

void enqueue_transaction(transaction txn) {
    pthread_mutex_lock(&queue_lock);

    // Add transaction to queue
    if (queue_size < MAX_QUEUE_SIZE) {
        transaction_queue[queue_size] = txn;
        queue_size++;
        pthread_cond_signal(&queue_cond); // Signal workers
    } else {
        // Handle queue overflow if necessary
        fprintf(stderr, "Transaction queue overflow\n");
    }

    pthread_mutex_unlock(&queue_lock);
}

transaction dequeue_transaction() {
    pthread_mutex_lock(&queue_lock);

    // Wait until there's a transaction in the queue or shutdown is signaled
    while (queue_size == 0 && !shutdown) {
        pthread_cond_wait(&queue_cond, &queue_lock);
    }

    transaction txn = {0};
    if (queue_size > 0) {
        txn = transaction_queue[0];
        memmove(transaction_queue, transaction_queue + 1, (queue_size - 1) * sizeof(transaction));
        queue_size--;
    }

    pthread_mutex_unlock(&queue_lock);
    return txn;
}

void* worker_thread(void* arg) {
    while (1) {
        transaction txn = dequeue_transaction();

        // Check for shutdown signal
        if (shutdown) break;

        // Process the transaction
        process_transaction(txn);

        pthread_mutex_lock(&queue_lock);
        transactions_processed++;
        pthread_mutex_unlock(&queue_lock);

        // Check for shutdown signal again after processing
        if (shutdown) break;
    }
    return NULL;
}

void* bank_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&queue_lock);
        if (shutdown) {
            pthread_mutex_unlock(&queue_lock);
            break;
        }
        pthread_mutex_unlock(&queue_lock);
        update_balance(NULL);
        sleep(1);
    }
    return NULL;
}

int process_transaction(transaction info) {
    char tType = info.tType;
    double amount = 0;

    // Lock the source account before modifying
    pthread_mutex_lock(&info.acc->ac_lock);
    printf("Locked source account %s\n", info.acc->account_number);

    if (tType == 'D') { // Deposit
        amount = info.amount;
        info.acc->transaction_tracker += amount;
        info.acc->balance += amount;
        printf("Deposited %.2f to account %s\n", amount, info.acc->account_number);
    } else if (tType == 'T') { // Transfer
        amount = info.amount;
        // Deduct from source account
        info.acc->balance -= amount;
        info.acc->transaction_tracker += amount;
        printf("Transferred %.2f from account %s\n", amount, info.acc->account_number);

        // Lock the target account before modifying
        pthread_mutex_lock(&info.target_acc->ac_lock);
        printf("Locked target account %s\n", info.target_acc->account_number);
        info.target_acc->balance += amount;
        printf("Transferred %.2f to account %s\n", amount, info.target_acc->account_number);
        pthread_mutex_unlock(&info.target_acc->ac_lock);
        printf("Unlocked target account %s\n", info.target_acc->account_number);
    } else if (tType == 'W') { // Withdraw
        amount = info.amount;
        info.acc->balance -= amount;
        info.acc->transaction_tracker += amount;
        printf("Withdrew %.2f from account %s\n", amount, info.acc->account_number);
    } else if (tType == 'C') { // Check balance
        FILE* out_file = fopen(info.acc->out_file, "a");
        if (out_file == NULL) {
            perror("Error opening file");
            pthread_mutex_unlock(&info.acc->ac_lock);
            printf("Unlocked source account %s\n", info.acc->account_number);
            return -1;
        }
        fprintf(out_file, "Current Savings Balance %.2f\n", 
                (info.acc->balance + info.acc->transaction_tracker));
        fclose(out_file);
        printf("Checked balance for account %s\n", info.acc->account_number);
    }

    // Unlock the source account after processing
    pthread_mutex_unlock(&info.acc->ac_lock);
    printf("Unlocked source account %s\n", info.acc->account_number);

    return 0;
}


void* update_balance(void* arg) {
    for (int i = 0; i < numAcc; i++) {
        pthread_mutex_lock(&accounts[i].ac_lock);

        // Update balance with reward rate
        accounts[i].balance += accounts[i].transaction_tracker * accounts[i].reward_rate;

        // Reset the transaction tracker
        accounts[i].transaction_tracker = 0;

        // Log the updated balance
        FILE* out_file = fopen(accounts[i].out_file, "a");
        if (out_file == NULL) {
            perror("Error opening account file");
        } else {
            fprintf(out_file, "Updated Savings Balance: %.2f\n", accounts[i].balance);
            fclose(out_file);
        }

        pthread_mutex_unlock(&accounts[i].ac_lock);
    }
    return NULL;
}
