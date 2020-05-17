#include "cpool.h"

void cpool::create_pools(int max_thread_num) {}

void cpool::add_task_to_pools(c_pool_t *pool, void *(*run)(void *), void *args) {
    c_work_t *work, *member;
    if (run == NULL) {
        char buf[1024];
        sprintf(buf, "%s is null", run);
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

void cpool::run(string args) {
    cout << "hello," << args << endl;
}