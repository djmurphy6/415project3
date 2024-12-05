#ifndef ACCOUNT_H_
#define ACCOUNT_H_
#include <pthread.h>
#include "string_parser.h"

extern int numAcc; // Declare the numAcc variable as an external variable
#define MAX_QUEUE_SIZE 1000

typedef struct
{
	char account_number[17];
	char password[9];
    double balance;
    double reward_rate;
    
    double transaction_tracker;

    char out_file[64];

    pthread_mutex_t ac_lock;
}account;

extern account* accounts; // Declare the accounts array as an external variablem

typedef struct 
{
    account* acc;
    char tType;
    double amount;
    account* target_acc;

}transaction;

extern pthread_mutex_t queue_lock;
extern pthread_cond_t queue_cond;
extern transaction* transaction_queue;
extern int queue_size;
extern int transactions_processed;
extern int total_transactions;
extern int shutdown;


int process_transaction (transaction info);
// function will be run by a worker thread to handle the transaction requests assigned to them

void* update_balance(void* arg);
//update each accounts balance based on their reward rate and transaction tracker

account* find_account(account* accounts, int numAcc, const char* account_number);

void enqueue_transaction(transaction txn);

transaction dequeue_transaction();

void* worker_thread(void* arg);

void* bank_thread(void* arg);


#endif /* ACCOUNT_H_ */