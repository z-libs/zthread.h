
#define ZTHREAD_IMPLEMENTATION
#define ZTHREAD_SHORT_NAMES
#include "zthread.h"
#include <stdio.h>

typedef struct 
{
    int data;
    int is_ready;
    mutex_t mutex;
    cond_t cond;
} SharedContext;

void producer(SharedContext *ctx)
{
    printf("[Producer] Generating data...\n");
    thread_sleep(1000); // We simulate heavy calculation.

    mutex_lock(&ctx->mutex);
    ctx->data = 42;
    ctx->is_ready = 1;
    printf("[Producer] Data ready! Signaling consumer.\n");
    
    cond_signal(&ctx->cond); // Wake up the waiting thread.
    mutex_unlock(&ctx->mutex);
}

void consumer(SharedContext *ctx) 
{
    printf("[Consumer] Waiting for data...\n");
    
    mutex_lock(&ctx->mutex);
    while (ctx->is_ready == 0) 
    {
        // zcond_wait releases the lock and sleeps. 
        // When signaled, it wakes up and reacquires the lock automatically.
        cond_wait(&ctx->cond, &ctx->mutex);
    }
    
    printf("[Consumer] Processed data: %d\n", ctx->data);
    mutex_unlock(&ctx->mutex);
}

int main(void) 
{
    SharedContext ctx = {0};
    mutex_init(&ctx.mutex);
    cond_init(&ctx.cond);

    zthread_t t_prod, t_cons;

    // Start consumer first, so it waits.
    thread_create(&t_cons, consumer, &ctx);
    thread_create(&t_prod, producer, &ctx);

    thread_join(t_prod);
    thread_join(t_cons);

    mutex_destroy(&ctx.mutex);
    cond_destroy(&ctx.cond);
    return 0;
}
