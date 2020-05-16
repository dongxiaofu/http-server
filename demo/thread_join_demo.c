#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct myarg_t {
    int x;
    int y;
} myarg_t;

typedef struct myret_t {
    int x;
    int y;
} myret_t;

void *my_thread(void *arg) {
    myarg_t *myarg = (myarg_t *) arg;
    printf("myarg->x = %d, myarg->y = %d\n", myarg->x, myarg->y);
    myret_t *myret = malloc(sizeof(myret_t));
    myret->x = 1;
    myret->y = 2;
    printf("myret x=%d,myret y=%d\n", myret->x, myret->y);
    return (void *) myret;
}

int main() {
    pthread_t pthread1;
    myret_t *myret;     // todo 容易出错，是*myret，而不是myret
    myarg_t myarg;
    int m;
    myarg.x = 5;
    myarg.y = 6;
    pthread_create(&pthread1, NULL, my_thread, &myarg);
    pthread_join(pthread1, (void **) &myret);
    printf("myret x=%d,myret y=%d\n", myret->x, myret->y);
    return 0;
}