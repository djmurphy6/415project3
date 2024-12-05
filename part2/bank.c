#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "account.h"
#include "string_parser.h"

#define NUM_WORKER_THREADS 10

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
transaction* transaction_queue = NULL;
int queue_size = 0;
int transactions_processed = 0;
int total_transactions = 0;
int shutdown = 0;

int numAcc = 0;
account* accounts = NULL;

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Open the input file
    FILE *inFPtr = fopen(argv[1], "r");
    if (inFPtr == NULL) {
        perror("Error opening input file");
        return 1;
    }

    // Allocate memory for reading lines
    size_t len = 128;
    char* line_buf = malloc(len);
    if (line_buf == NULL) {
        perror("Error allocating memory for line buffer");
        fclose(inFPtr);
        return 1;
    }

    // Read the number of accounts
    if (getline(&line_buf, &len, inFPtr) == -1) {
        perror("Error reading the number of accounts");
        free(line_buf);
        fclose(inFPtr);
        return 1;
    }
    numAcc = atoi(line_buf);

    // Allocate memory for accounts
    accounts = malloc(numAcc * sizeof(account));
    if (accounts == NULL) {
        perror("Error allocating memory for accounts");
        free(line_buf);
        fclose(inFPtr);
        return 1;
    }

    // Initialize accounts
    for (int i = 0; i < numAcc; i++) {
        if (getline(&line_buf, &len, inFPtr) == -1) {
            perror("Error reading account details");
            free(accounts);
            free(line_buf);
            fclose(inFPtr);
            return 1;
        }

        // Read account details
        for (int j = 0; j < 4; j++) {
            if (getline(&line_buf, &len, inFPtr) == -1) {
                perror("Error reading account details");
                free(accounts);
                free(line_buf);
                fclose(inFPtr);
                return 1;
            }

            command_line tokens = str_filler(line_buf, "\n");
            if (j == 0) strncpy(accounts[i].account_number, tokens.command_list[0], 16);
            if (j == 1) strncpy(accounts[i].password, tokens.command_list[0], 8);
            if (j == 2) accounts[i].balance = atof(tokens.command_list[0]);
            if (j == 3) accounts[i].reward_rate = atof(tokens.command_list[0]);
            free_command_line(&tokens);
        }

        snprintf(accounts[i].out_file, sizeof(accounts[i].out_file), "account_%d.txt", i);
        pthread_mutex_init(&accounts[i].ac_lock, NULL);

        // Write initial account balance
        FILE* outFPtr = fopen(accounts[i].out_file, "w");
        if (outFPtr != NULL) {
            fprintf(outFPtr, "account: %d\nCurrent Savings Balance %.2f\n", i, accounts[i].balance);
            fclose(outFPtr);
        }
    }

    // Create worker threads
    pthread_t workers[NUM_WORKER_THREADS];
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        pthread_create(&workers[i], NULL, worker_thread, NULL);
    }

    // Read transactions from the file
    while (getline(&line_buf, &len, inFPtr) != -1) {
        command_line large_token_buffer = str_filler(line_buf, "\n");
        command_line tokens = str_filler(large_token_buffer.command_list[0], " ");
        transaction txn = {0};

        char* account_number = tokens.command_list[1];
        char* password = tokens.command_list[2];
        account* acc = find_account(accounts, numAcc, account_number);

        if (acc == NULL || strcmp(acc->password, password) != 0) {
            free_command_line(&tokens);
            free_command_line(&large_token_buffer);
            continue;
        }

        txn.acc = acc;
        txn.tType = tokens.command_list[0][0];
        if (txn.tType == 'D') txn.amount = atof(tokens.command_list[3]);
        if (txn.tType == 'T') {
            txn.target_acc = find_account(accounts, numAcc, tokens.command_list[3]);
            txn.amount = atof(tokens.command_list[4]);
        }
        if (txn.tType == 'W') txn.amount = atof(tokens.command_list[3]);

        enqueue_transaction(txn);
        total_transactions++;

        free_command_line(&tokens);
        free_command_line(&large_token_buffer);
    }

    // Create the bank thread
    pthread_t bank;
    pthread_create(&bank, NULL, bank_thread, NULL);

    // Wait for worker threads to finish
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        pthread_join(workers[i], NULL);
    }

    // Signal shutdown for the bank thread
    pthread_mutex_lock(&queue_lock);
    shutdown = 1;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_lock);

    pthread_join(bank, NULL);

    // Write summary output
    FILE* summaryFPtr = fopen("output.txt", "w");
    if (summaryFPtr != NULL) {
        for (int i = 0; i < numAcc; i++) {
            fprintf(summaryFPtr, "%d balance: %.2f\n", i, accounts[i].balance);
        }
        fclose(summaryFPtr);
    }

    // Cleanup
    fclose(inFPtr);
    free(line_buf);
    free(accounts);
    free(transaction_queue);
    pthread_mutex_destroy(&queue_lock);
    pthread_cond_destroy(&queue_cond);

    printf("End of file\nBye Bye\n");
    return 0;
}