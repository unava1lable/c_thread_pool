#include "thread_pool.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int errno;

static void *thread_work(void * args) {
    pool_t *pool = (pool_t *)args;
    pool_work_t *work = NULL;

    for (;;) {
        pthread_mutex_lock(&pool->queue_lock);
        while (!pool->pool_head && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_ready, &pool->queue_lock);
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_lock);
            pthread_exit(NULL);
        }

        work = pool->pool_head;
        pool->pool_head = (pool_work_t *)pool->pool_head->next;
        pthread_mutex_unlock(&pool->queue_lock);
        work->work(work->args);

        free(work);
    }
    return NULL;
}

int pool_create(pool_t *pool, size_t max_size) {
    pool = (pool_t *)malloc(sizeof(pool_t));
    if (pool == NULL) {
        return -1;
    }
    pool->shutdown = 0;
    pool->max_thread = max_size;
    pool->thread_id = (pthread_t *)malloc(sizeof(pthread_t) * pool->max_thread);
    if (pool->thread_id == NULL) {
        return -1;
    }
    pool->pool_head = NULL;
    if (pthread_mutex_init(&pool->queue_lock, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&pool->queue_ready, NULL) != 0) {
        return -1;
    }

    for (int i = 0; i < pool->max_thread; i++) {
        if (pthread_create(&(pool->thread_id[i]), NULL, thread_work, (void *)pool) != 0) {
            return -1;
        }        
    }
    return 0;
}

void pool_destroy(pool_t *pool) {
    pool_work_t *temp_work;

    if (pool->shutdown) {
        return;
    }
    pool->shutdown = 1;

    pthread_mutex_lock(&pool->queue_lock);
    pthread_cond_broadcast(&pool->queue_ready);
    pthread_mutex_unlock(&pool->queue_lock);

    for (int i = 0; i < pool->max_thread; i++) {
        pthread_join(pool->thread_id[i], NULL);
    }

    free(pool->thread_id);
    while (pool->pool_head) {
        temp_work = pool->pool_head;
        pool->pool_head = (pool_work_t *)pool->pool_head->next;
        free(temp_work);
    }
    pthread_mutex_destroy(&pool->queue_lock);
    pthread_cond_destroy(&pool->queue_ready);
    free(pool);
}