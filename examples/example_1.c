
#define ZTHREAD_IMPLEMENTATION
#define ZTHREAD_SHORT_NAMES
#include "zthread.h"
#include <stdio.h>

typedef struct 
{
    int balance;
    mutex_t lock;
} BankAccount;

void deposit_task(BankAccount *acct) 
{
    for (int i = 0; i < 100000; i++) 
    {
        mutex_lock(&acct->lock);
        acct->balance += 1; // Critical section.
        mutex_unlock(&acct->lock);
    }
}

int main(void) 
{
    BankAccount my_account = {0};
    mutex_init(&my_account.lock);

    zthread_t t1, t2;

    printf("Starting balance: %d\n", my_account.balance);
    printf("Depositing 200,000 via two threads...\n");
    
    // Two threads fighting over the same memory.
    thread_create(&t1, deposit_task, &my_account);
    thread_create(&t2, deposit_task, &my_account);

    thread_join(t1);
    thread_join(t2);

    // Should be exactly 200000. Without mutex, it would be random.
    printf("Final balance:    %d\n", my_account.balance);

    mutex_destroy(&my_account.lock);
    return 0;
}
