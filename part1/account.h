#ifndef ACCOUNT_H_
#define ACCOUNT_H_
#include <pthread.h>
#include "string_parser.h"

extern int numAcc; // Declare the numAcc variable as an external variable

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


int process_transaction (transaction info);
// function will be run by a worker thread to handle the transaction requests assigned to them

int update_balance ();
//update each accounts balance based on their reward rate and transaction tracker

account* find_account(account* accounts, int numAcc, const char* account_number);


#endif /* ACCOUNT_H_ */