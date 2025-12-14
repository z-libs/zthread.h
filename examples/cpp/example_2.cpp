
#define ZTHREAD_IMPLEMENTATION
#include "zthread.h"
#include <iostream>
#include <queue>

class WorkQueue 
{
    std::queue<int> items;
    z_thread::mutex mtx;
    z_thread::cond cv;
    bool finished = false;

public:
    void push(int item) 
    {
        z_thread::lock_guard lock(mtx);
        items.push(item);
        cv.signal(); // Wake one worker.
    }

    void finish() 
    {
        z_thread::lock_guard lock(mtx);
        finished = true;
        cv.broadcast(); // Wake everyone.
    }

    // Returns true if got item, false if finished.
    bool pop(int &out_item) 
    {
        z_thread::lock_guard lock(mtx);
        
        while(items.empty() && !finished) 
        {
            cv.wait(mtx);
        }

        if(items.empty() && finished) return false;

        out_item = items.front();
        items.pop();
        return true;
    }
};

int main() 
{
    WorkQueue queue;

    // Consumer.
    z_thread::thread worker([&]{
        int job;
        while(queue.pop(job)) 
        {
            std::cout << "Processing Job #" << job << "\n";
        }
        std::cout << "Worker finished.\n";
    });

    // Producer.
    for(int i=1; i<=5; ++i) 
    {
        std::cout << "Pushing " << i << "...\n";
        queue.push(i);
        z_thread::thread::sleep(50);
    }

    queue.finish();
    worker.join();
    return 0;
}
