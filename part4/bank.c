#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include "account.h"
#include "string_parser.h"

int numAcc = 0;
account* accounts = NULL;
int counter = 0;
int total_transactions = 0;
int transactions_processed = 0;
int done = 0;

pthread_t thread_ids[NUM_WORKERS];
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t counter_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t bank_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bank_cond = PTHREAD_COND_INITIALIZER;

pthread_barrier_t barrier;

FILE* inFPtr;

void* worker_thread(void* arg);

int main(int argc, char const* argv[]) {
    if (argc == 2) {
        inFPtr = fopen(argv[1], "r");
        if (inFPtr == NULL) {
            perror("Error opening file");
            return 1;
        }

        size_t len = 128;
        char* line_buf = malloc(len);
        if (line_buf == NULL) {
            perror("Error allocating memory for line_buf");
            fclose(inFPtr);
            return 1;
        }

        system("mkdir -p Output");

        if (getline(&line_buf, &len, inFPtr) != -1) {
            numAcc = atoi(line_buf);
        } else {
            perror("Error reading the number of accounts");
            fclose(inFPtr);
            free(line_buf);
            return 1;
        }

        accounts = malloc(numAcc * sizeof(account));
        if (accounts == NULL) {
            perror("Error allocating memory for accounts");
            fclose(inFPtr);
            free(line_buf);
            return 1;
        }

        // Read account data
        // Initialize accounts from input file...
        
        pthread_t bank_thread_id;
        pthread_create(&bank_thread_id, NULL, update_balance, NULL); // Bank thread
        
        for (int i = 0; i < NUM_WORKERS; i++) {
            pthread_create(&thread_ids[i], NULL, worker_thread, NULL);
        }

        // Handle transactions and account updates...
        // Wait for threads to join...

        pthread_join(bank_thread_id, NULL);

        // Write summary to file
        FILE* summaryFPtr = fopen("output.txt", "w");
        if (summaryFPtr == NULL) {
            perror("Error opening summary output file");
            fclose(inFPtr);
            free(line_buf);
            free(accounts);
            return 1;
        }

        // Write the summary information
        for (int i = 0; i < numAcc; i++) {
            fprintf(summaryFPtr, "%d balance:  %.2f\n\n", i, accounts[i].balance);
        }
        fclose(summaryFPtr);

        fclose(inFPtr);
        free(line_buf);
        free(accounts);
    }

    return 0;
}

void* worker_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&queue_lock);

        // Wait for a transaction if the queue is empty
        while (queue_size == 0 && !done) {
            //printf("Queue empty - Worker thread waiting\n");
            pthread_cond_wait(&queue_cond, &queue_lock);
        }

        // Exit if done and queue is empty
        if (done && queue_size == 0) {
            pthread_mutex_unlock(&queue_lock);
            break;
        }

        // Fetch a transaction from the queue
        transaction txn = transaction_queue[0];
        memmove(transaction_queue, transaction_queue + 1, (queue_size - 1) * sizeof(transaction));
        queue_size--;
        pthread_mutex_unlock(&queue_lock);

        // Process the transaction
        process_transaction(&txn);

        // Update counters and signal the bank thread if needed
        pthread_mutex_lock(&counter_lock);
        if (counter >= TRANSACTIONS_THRESHOLD) {
            pthread_cond_signal(&bank_cond); // Notify `update_balance` for balance update
        }
        pthread_mutex_unlock(&counter_lock);

        // Wait at the barrier after processing a transaction
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}