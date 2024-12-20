#ifndef ACCOUNT_H_
#define ACCOUNT_H_
#include <pthread.h>

#define NUM_WORKERS 10
#define INITIAL_QUEUE_SIZE 10
#define TRANSACTIONS_THRESHOLD 5000

extern int numAcc; // Declare the numAcc variable as an external variable
extern int counter; // Declare the counter variable as an external variable

extern pthread_mutex_t counter_lock;
extern pthread_mutex_t bank_lock;

extern pthread_cond_t counter_cond;
extern pthread_cond_t bank_cond;

extern int done; // flag to see if all transactions are done
extern int transactions_processed;
extern int total_transactions;

typedef struct {
    char account_number[17];
    char password[9];
    double balance;
    double reward_rate;
    double transaction_tracker;
    pthread_mutex_t ac_lock;
} account;

extern account* accounts; // Declare the accounts array as an external variable

typedef struct {
    account* acc;
    char tType;
    double amount;
    account* target_acc;
} transaction;

void* process_transaction(void* arg);
void* update_balance(void* arg);
account* find_account(account* accounts, int numAcc, const char* account_number);

#endif /* ACCOUNT_H_ */