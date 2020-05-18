#include "cpool.h"

// todo 为什么要用**？
void cpool::create_pools(c_pool_t **pools, int max_thread_num) {
    (*pools) = (c_pool_t *) malloc(sizeof(c_pool_t) * max_thread_num);
    (*pools)->max_thread_num = max_thread_num;
    (*pools)->shutdown = 0;
    (*pools)->pthread_id = (pthread_t *) malloc(sizeof(pthread_t) * max_thread_num);
    (*pools)->head = NULL;
    pthread_mutex_init(((*pools)->queue_lock), NULL);
    pthread_cond_init((*pools)->queue_ready, NULL);

    for (int i = 0; i < max_thread_num; i++) {
//        void *str = (void *) "hello";
        // todo 第一个参数，很奇怪
        if (pthread_create(&((*pools)->pthread_id[i]), NULL, run, (*pools)) != 0) {
            printf("pthread_create failed!\n");
            exit(-1);
        }
    }
}

void cpool::add_task_to_pools(c_pool_t *pool, void *(*run)(void *), void *args) {
    c_work_t *work, *member;
    if (run == NULL) {
        char buf[1024];
        sprintf(buf, "run is null");
        perror(buf);
        exit(1);
    }

    work = new c_work_t;
    work->run = run;
    work->args = args;
    work->next = NULL;

    // 加锁
    pthread_mutex_lock(pool->queue_lock);
    member = pool->head;
    if (member == NULL) {
        pool->head = work;
    } else {
        while (member != NULL) {
            member = member->next;
        }
        member->next = work;
    }

    pthread_cond_signal(pool->queue_ready);
    pthread_mutex_unlock(pool->queue_lock);
}

void * run(void *args) {
    c_pool_t *pools=(c_pool_t *)args;
    c_work_t *work=NULL;

    while(1){
        pthread_mutex_lock(pools->queue_lock);
        while(!pools->head && !pools->shutdown){
            pthread_cond_wait(pools->queue_ready,pools->queue_lock);
        }

        if(pools->shutdown){
            pthread_mutex_unlock(pools->queue_lock);
            pthread_exit(NULL);
        }

        work=pools->head;
        pools->head=work->next;
        pthread_mutex_unlock(pools->queue_lock);
        work->run(args);
        free(work);
    }

    return NULL;
}