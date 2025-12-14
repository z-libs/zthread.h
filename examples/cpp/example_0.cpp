
#define ZTHREAD_IMPLEMENTATION
#include "zthread.h"
#include <iostream>
#include <vector>

int main() 
{
    z_thread::thread t1([]{
        std::cout << "Hello from Lambda Thread!\n";
    });

    int x = 42;
    z_thread::thread t2([x]{
        std::cout << "Captured value: " << x << "\n";
    });

    std::vector<z_thread::thread> workers;
    for(int i = 0; i < 3; i++) 
    {
        workers.emplace_back([i]{
            z_thread::thread::sleep(100);
            std::cout << "Worker " << i << " done.\n";
        });
    }

    t1.join();
    t2.join();
    for(auto& t : workers) t.join();

    return 0;
}
