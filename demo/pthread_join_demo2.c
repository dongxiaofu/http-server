#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *my_thread(void *arg) {
    sleep(3);
    int m = (int) arg;
    printf("m = %d\n", m);
    return (void *) (arg + 1);
}

int main() {
    int m = 5;
    int r;
    pthread_t p;
    pthread_create(&p, NULL, my_thread, (void *) 100);
    printf("k = %d\n", 3);
    pthread_join(p, (void **) &r);
    printf("r = %d\n", r);
    return 0;
}