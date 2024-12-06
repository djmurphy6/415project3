#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"

int transactions = 0;

int numAcc = 0;
account* accounts = NULL;

#define NUM_WORKERS 10
#define INITIAL_QUEUE_SIZE 10

transaction* transaction_queue = NULL;
int queue_size = 0;
int max_queue_size = INITIAL_QUEUE_SIZE;

int done = 0;



pthread_t thread_ids[NUM_WORKERS];
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// Instantiate file pointer
FILE *inFPtr;

void* worker_thread(void* arg);

int main(int argc, char const *argv[]){
    if(argc == 2){ //checking for command line argument && (strcmp(argv[1], "f") == 0)
        //opening file to read
        inFPtr = fopen (argv[1], "r");
        if(inFPtr == NULL) {
            printf("Error opening file");
            return 1;
        }

        //declare line_buffer
        size_t len = 128;
        char* line_buf = malloc (len);
        if (line_buf == NULL) {
            printf("Error allocating memory for line_buf");
            fclose(inFPtr); // Close the file 
            return 1;
        }

        int line_num = 0;

        // Read the first line to get the number of accounts
        if (getline(&line_buf, &len, inFPtr) != -1) {
            numAcc = atoi(line_buf); // Convert the first line to an integer
            line_num++;
        } else {
            printf("Error reading the number of accounts");
            fclose(inFPtr); // Close the file
            free(line_buf); // Free the line buffer
            return 1;
        }

        command_line large_token_buffer;

        accounts = malloc(numAcc * sizeof(account));
        if (accounts == NULL) {
            printf("Error allocating memory for accounts\n");
            fclose(inFPtr); // Close the file
            free(line_buf); // Free the line buffer
            return 1;
        }
        

        for (int i = 0; i < numAcc; i++) {
            // Skip the index line (line 1 for each account)
            getline(&line_buf, &len, inFPtr);
            // printf("Account %d index line: %s", i, line_buf);

            for (int j = 0; j < 4; j++) {
                getline(&line_buf, &len, inFPtr);
                large_token_buffer = str_filler(line_buf, "\n");

                // Assign the parsed values to the appropriate struct fields
                if (j == 0) { 
                    // Account number
                    strncpy(accounts[i].account_number, large_token_buffer.command_list[0], 16);
                    accounts[i].account_number[16] = '\0'; // Null-terminate
                    
                    //printf("Account %d - Account Number: %s\n", i + 1, accounts[i].account_number);
                } else if (j == 1) {
                    // Password
                    strncpy(accounts[i].password, large_token_buffer.command_list[0], 8);
                    accounts[i].password[8] = '\0'; // Null-terminate
                    
                    //printf("Account %d - Password: %s\n", i + 1, accounts[i].password);
                } else if (j == 2) {
                    // Balance
                    accounts[i].balance = atof(large_token_buffer.command_list[0]); // Convert to double
                    
                    //printf("Account %d - Balance: %.2f\n", i + 1, accounts[i].balance);
                } else if (j == 3) {
                    // Reward rate
                    accounts[i].reward_rate = atof(large_token_buffer.command_list[0]); // Convert to double
                    
                    //printf("Account %d - Reward Rate: %.3f\n", i + 1, accounts[i].reward_rate);
                }

                //initialize mutex lock
                pthread_mutex_init(&accounts[i].ac_lock, NULL);

                // Free memory for the token buffer
                free_command_line(&large_token_buffer);
            }
        }

        int dCt = 0;
        int tCt = 0;
        int wCt = 0;
        int cCt = 0;
        int invalid=0;

        // Initialize the transaction queue
        transaction_queue = malloc(INITIAL_QUEUE_SIZE * sizeof(transaction));
        if (transaction_queue == NULL) {
            perror("Failed to allocate memory for transaction queue");
            exit(EXIT_FAILURE);
        }

        // Create worker threads
        for (int i = 0; i < NUM_WORKERS; i++) {
            pthread_create(&thread_ids[i], NULL, worker_thread, NULL);
        }
        
        
        while(getline(&line_buf, &len, inFPtr) != -1){
            //memory leak here
            large_token_buffer = str_filler(line_buf, "\n");

            //debug code
            //printf("Large Token Buffer: %s\n", large_token_buffer.command_list[0]);

            transaction txn = {0};
            //memory leak here
            command_line tokens = str_filler(large_token_buffer.command_list[0], " ");

            char *account_number = tokens.command_list[1];
            char *password = tokens.command_list[2];

            // find account associated with the account number
            account* acc = find_account(accounts, numAcc, account_number);

            // check if account exists and password matches
            if (acc == NULL || strcmp(acc->password, password) != 0) {
                //printf("Invalid account or password for account %s\n", account_number);
                invalid++;
                free_command_line(&tokens);
                free_command_line(&large_token_buffer);
                // skip to next line
                continue;
            } else {
                txn.acc = acc;
                txn.tType = tokens.command_list[0][0];
                txn.amount = 0;
                txn.target_acc = NULL;

                if(strcmp(tokens.command_list[0], "D") == 0){
                    // DEPOSIT - has 3 tokens, D account_num password amount
                    dCt++;
                    txn.amount = atof(tokens.command_list[3]);
                }
                else if(strcmp(tokens.command_list[0], "T") == 0){
                    // TRANSFER - has 4 tokens, T src_account password dest_account transfer_amount
                    tCt++;

                    account *found_account = find_account(accounts, numAcc, tokens.command_list[3]);
                    if (found_account == NULL) {
                        //printf("Error: Target account not found\n");
                        continue;
                    }
                    txn.target_acc = found_account;
                    txn.amount = atof(tokens.command_list[4]);
                }
                else if(strcmp(tokens.command_list[0], "W") == 0){
                    // WITHDRAW - has 3 tokens, W account_num password withdraw_amount
                    wCt++;
                    txn.amount = atof(tokens.command_list[3]);      
                }
                else if(strcmp(tokens.command_list[0], "C") == 0){
                    // CHECK BALANCE - has 2 tokens, C account_num password
                    cCt++;
                    //printf("Account %s balance: %.2f\n", account_number, acc.balance);

                }
                     
            }
            pthread_mutex_lock(&queue_lock);
            if (queue_size >= max_queue_size) {
                max_queue_size *= 2;
                transaction_queue = realloc(transaction_queue, max_queue_size * sizeof(transaction));
                if (transaction_queue == NULL) {
                    perror("Failed to reallocate memory for transaction queue");
                    exit(EXIT_FAILURE);
                }
            }
            transaction_queue[queue_size] = txn;
            queue_size++;
            pthread_cond_signal(&queue_cond);
            pthread_mutex_unlock(&queue_lock);

            // free memory
            free_command_line(&tokens);
            free_command_line(&large_token_buffer);
        }

        pthread_mutex_lock(&queue_lock);
        done = 1; // Signal that no more transactions will be added
        pthread_cond_broadcast(&queue_cond); // Wake up all waiting threads
        pthread_mutex_unlock(&queue_lock);

        for (int j = 0; j < NUM_WORKERS; ++j){
            printf("Joining thread %d\n", j);
            pthread_join(thread_ids[j], NULL);			// wait on our threads to rejoin main thread
        }

        printf("Total Transactions: %d\n", transactions);


        update_balance(NULL);

        // Open the output.txt file for writing
        FILE *summaryFPtr = fopen("output.txt", "w");
        if (summaryFPtr == NULL) {
            printf("Error opening summary output file\n");
            fclose(inFPtr);
            free(line_buf);
            free(accounts);
            return 1;
        }

        // Write the summary information to the output.txt file
        for(int i = 0; i < numAcc; i++) {
            fprintf(summaryFPtr, "%d balance:  %.2f\n\n", i, accounts[i].balance);
        }


        // Close the summary output file
        fclose(summaryFPtr);

        //debug code
        /**
        printf("Transfer Count: %d\n", tCt);
        printf("Withdraw Count: %d\n", wCt);
        printf("Check Balance Count: %d\n", cCt);
        printf("Invalid Count: %d\n", invalid);
        */

        fclose(inFPtr);
        //free line buffer
        free (line_buf);
        printf("End of file\nBye Bye\n");
        free(accounts);
        
    }
    else{
        printf("Invalid number of arguments\n");
        return 1;
    }
}

void* worker_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&queue_lock);
        while (queue_size == 0 && !done) {
            pthread_cond_wait(&queue_cond, &queue_lock);
        }

        if (queue_size == 0 && done) {
            pthread_mutex_unlock(&queue_lock);
            break; // Exit the thread if there are no more transactions and we're done
        }

        // Get the next transaction from the queue
        transaction txn = transaction_queue[0];
        memmove(transaction_queue, transaction_queue + 1, (queue_size - 1) * sizeof(transaction));
        queue_size--;
        pthread_mutex_unlock(&queue_lock);

        // Process the transaction
        transactions++;
        process_transaction(&txn);
    }
    return NULL;
}


