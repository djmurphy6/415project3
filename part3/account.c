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

    // Synchronize threads using the barrier
        //pthread_barrier_wait(&barrier);
    
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
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'T') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        info->target_acc->balance += amount;
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'W') {
        amount = info->amount;
        info->acc->balance -= amount;
        info->acc->transaction_tracker += amount;
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);

    } else if(tType == 'C') {
        pthread_mutex_lock(&pipe_lock);
        numCheck++;
        if(numCheck % 500 == 0) {
            printf("Numcheck: %d\n", numCheck);
            time_t current_time = time(NULL);
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                    "Worker checked balance of account %s. Balance is $%.2f. Check occurred at %s",
                    info->acc->account_number, info->acc->balance, ctime(&current_time));
            write(pipe_fd[1], buffer, strlen(buffer)); // Use write() for pipes
        }
        pthread_mutex_unlock(&pipe_lock);
    }
    
    if(tType != 'C') {
        // Print counter
        /**
        pthread_mutex_lock(&counter_lock);
            printf("Counter: %d\n", counter);
        pthread_mutex_unlock(&counter_lock);
        */
    }

    // Unlock account after processing transaction
    pthread_mutex_unlock(&info->acc->ac_lock);
    return NULL;
}


void* update_balance(void* arg) {
    // Synchronize threads using the barrier
        //pthread_barrier_wait(&barrier);
    //printf("Bank thread started\n");
    pthread_mutex_lock(&counter_lock);
    for (int i = 0; i < 10; i++) {
        char filename[20];
        sprintf(filename, "account_%d.txt", i);
        FILE *file = fopen(filename, "w");
        if (file != NULL) {
            fprintf(file, "account %d:\n", i);
            fclose(file);
        } else {
            perror("Error opening file");
        }
    }
    while (!done) {
        pthread_mutex_lock(&counter_lock);
        while (counter < TRANSACTIONS_THRESHOLD && !done) {
            pthread_cond_wait(&bank_cond, &counter_lock);
        }
        pthread_mutex_unlock(&counter_lock);

        if ((done && counter == 0) || transactions_processed >= 90000) break;


        // Exit the loop if processing is complete
        /**
        if (done && transactions_processed >= total_transactions) {
            pthread_mutex_unlock(&counter_lock);
            break;
        }
        */

        // Perform the balance update
        for (int i = 0; i < numAcc; i++) {
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].balance += (accounts[i].transaction_tracker * accounts[i].reward_rate);
            accounts[i].transaction_tracker = 0;
            char filename[20];
            sprintf(filename, "account_%d.txt", i);
            FILE *file = fopen(filename, "a");
            if (file != NULL) {
                fprintf(file, "Current Balance:\t%.2f\n", accounts[i].balance);
                fclose(file);
            } else {
                perror("Error opening file");
            }
            pthread_mutex_unlock(&accounts[i].ac_lock);
        }

        printf("Transactions_processed: %d\n", transactions_processed);



        // Write final balances to pipe
        char buffer[256];
        for (int i = 0; i < numAcc; i++) {
            snprintf(buffer, sizeof(buffer), "Final balance of account %s: %.2f\n", accounts[i].account_number, accounts[i].balance);
            write(pipe_fd[1], buffer, strlen(buffer));
        }

        // Reset counter and allow worker threads to continue
        transactions_processed += counter;
        printf("Counter Reset\n");
        counter = 0;
        
    }

    return NULL;
}

