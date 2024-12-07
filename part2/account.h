#ifndef ACCOUNT_H_
#define ACCOUNT_H_
#include <pthread.h>
#include "string_parser.h"

#define NUM_WORKERS 10

extern int numAcc; // Declare the numAcc variable as an external variable

// for audit
extern int numCheck; // Declare the numCheck variable as an external variable
extern pthread_mutex_t pipe_lock;
extern int pipe_fd[2];

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


void* process_transaction (void* arg);
// function will be run by a worker thread to handle the transaction requests assigned to them

void* update_balance(void* arg);
//update each accounts balance based on their reward rate and transaction tracker

account* find_account(account* accounts, int numAcc, const char* account_number);


#endif /* ACCOUNT_H_ */