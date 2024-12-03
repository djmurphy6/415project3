#ifndef ACCOUNT_H_
#define ACCOUNT_H_
#include <pthread.h>

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

void* process_transaction (void* arg);
// function will be run by a worker thread to handle the transaction requests assigned to them

void* update_balance (void* arg);
//update each accounts balance based on their reward rate and transaction tracker


#endif /* ACCOUNT_H_ */