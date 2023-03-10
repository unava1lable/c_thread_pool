#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#endif

#include <pthread.h>

typedef struct pool_work {
    void *(*work)(void *);
    void *args;
    struct pool_work *next;
} pool_work_t;

typedef struct pool {
    pthread_t *thread_id;
    pool_work_t *pool_head;
    pthread_cond_t queue_ready;
    pthread_mutex_t queue_lock;
    size_t max_thread;
    int shutdown;
} pool_t;

int pool_create(pool_t **pool, size_t max_thread);
void pool_destroy(pool_t *pool);
int pool_add_tast(pool_t *pool, void *(*work)(void *), void *arg);