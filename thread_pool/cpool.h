#include <pthread.h>
#include <string>
#include <iostream>

using namespace std;

typedef struct c_work {
    void *(*run)(void *);

    void *args;
    struct c_work *next;
} c_work_t;

typedef struct c_pool {
    int max_thread_num;  // 线程池拥有的最大线程数
    pthread_t *pthread_id;   // 线程ID数组
    bool shutdown;   // 线程池是否关闭
    pthread_cond_t *queue_ready;  // 条件变量
    pthread_mutex_t *queue_lock;
    c_work_t *head;
} c_pool_t;

void * run(void *args);

class cpool {
public:
    void create_pools(c_pool_t **pools, int max_thread_num);

    void add_task_to_pools(c_pool_t *pool, void *(*run)(void *), void *args);
};