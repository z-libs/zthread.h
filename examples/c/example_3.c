
#define ZTHREAD_IMPLEMENTATION
#define ZTHREAD_SHORT_NAMES
#include "zthread.h"
#include <stdio.h>
#include <stdbool.h>

#define MAX_JOBS 5

typedef struct 
{
    int items[MAX_JOBS];
    int count;
    int head;
    int tail;
    bool done;          // Flag to tell workers to stop.
    mutex_t lock;       // Protects the queue.
    cond_t  has_work;   // Signals "Queue is not empty".
    cond_t  has_space;  // Signals "Queue is not full".
} JobQueue;

void queue_init(JobQueue *q) 
{
    q->count = 0; 
    q->head = 0; 
    q->tail = 0; 
    q->done = false;
    mutex_init(&q->lock);
    cond_init(&q->has_work);
    cond_init(&q->has_space);
}

// Producer: Adds a job (blocking if full).
void queue_push(JobQueue *q, int job_id) 
{
    mutex_lock(&q->lock);
    
    while (q->count == MAX_JOBS) 
    {
        cond_wait(&q->has_space, &q->lock);
    }
    
    q->items[q->tail] = job_id;
    q->tail = (q->tail + 1) % MAX_JOBS;
    q->count++;
    
    printf("[Producer] Added Job #%d (Count: %d)\n", job_id, q->count);
    
    cond_signal(&q->has_work); // Wake up a worker.
    mutex_unlock(&q->lock);
}

// Consumer: Worker thread function.
void worker(JobQueue *q) 
{
    while (true) 
    {
        mutex_lock(&q->lock);
        
        while (q->count == 0 && !q->done) 
        {
            cond_wait(&q->has_work, &q->lock);
        }

        if (q->count == 0 && q->done) 
        {
            mutex_unlock(&q->lock);
            break;
        }

        int job = q->items[q->head];
        q->head = (q->head + 1) % MAX_JOBS;
        q->count--;
        
        cond_signal(&q->has_space);
        mutex_unlock(&q->lock);

        // Process job (outside lock).
        printf("    => [Worker] Processing Job #%d...\n", job);
        thread_sleep(100); // We simulate the work, this is just a demo.
    }
}

int main(void) 
{
    JobQueue queue;
    queue_init(&queue);

    printf("=> Work queue demo.\n");

    thread_t w1, w2;
    thread_create(&w1, worker, &queue);
    thread_create(&w2, worker, &queue);

    for (int i = 1; i <= 10; i++) 
    {
        queue_push(&queue, i);
        thread_sleep(50);
    }

    mutex_lock(&queue.lock);
    queue.done = true;
    cond_broadcast(&queue.has_work);
    mutex_unlock(&queue.lock);

    thread_join(w1);
    thread_join(w2);

    printf("=> All jobs processed.\n");
    return 0;
}
