
#define ZTHREAD_IMPLEMENTATION
#include "zthread.h"
#include <iostream>

struct BankAccount 
{
    int balance = 0;
    z_thread::mutex mtx;

    void deposit(int amount) 
    {
        z_thread::lock_guard lock(mtx);
        
        balance += amount;
        // mtx is unlocked here.
    }
};

int main() 
{
    BankAccount account;
    
    z_thread::thread t1([&]{ 
        for(int i=0; i<1000; ++i) account.deposit(1); 
    });

    z_thread::thread t2([&]{ 
        for(int i=0; i<1000; ++i) account.deposit(1); 
    });

    t1.join();
    t2.join();

    std::cout << "Final Balance: " << account.balance << " (Expected: 2000)\n";
    return 0;
}
