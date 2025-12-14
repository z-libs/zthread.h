
#define ZTHREAD_IMPLEMENTATION
#define ZTHREAD_SHORT_NAMES
#include "zthread.h"
#include <stdio.h>

// Takes a typed pointer, no void* casting needed!
void worker(int *id) 
{
    printf("Thread %d starting work...\n", *id);
    thread_sleep(500); // We simulate a expensive job.
    printf("Thread %d done.\n", *id);
}

int main(void) 
{
    zthread_t threads[3];
    int ids[3] = {1, 2, 3};

    printf("=> Spawning 3 threads\n");

    for (int i = 0; i < 3; i++) 
    {
        // thread_create automatically handles the type casting.
        thread_create(&threads[i], worker, &ids[i]);
    }

    // Wait for all to finish.
    for (int i = 0; i < 3; i++) 
    {
        thread_join(threads[i]);
    }

    printf("=> All Threads Finished\n");
    return 0;
}
