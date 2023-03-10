#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#endif

#include <pthread.h>

typedef struct pool_work {
    void *(*work)(void *);   // work function
    void *args;              // args
    struct pool_work *next;
} pool_work_t;

typedef struct pool {
    pthread_t *thread_id;        // array of threads
    pool_work_t *pool_head;      // pool_work queue
    pthread_cond_t queue_ready;  // condition variable
    pthread_mutex_t queue_lock;  // queue lock
    size_t max_thread;           // maximum of threads
    int shutdown;                // 0 for false, 1 for true
} pool_t;


// create a pool
// return 0 for success; -1 for failure
int pool_create(pool_t **pool, size_t max_thread);

// destroy the pool;
void pool_destroy(pool_t *pool);

// add a new task to the pool
// return 0 for success; -1 for failure
int pool_add_task(pool_t *pool, void *(*work)(void *), void *args);