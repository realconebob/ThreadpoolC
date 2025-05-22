#include "../threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tcb0(void *data) {
    if(!data) {return -1;}
    printf("tcb0: %s\n", (char*)data);
    return 0;
}
static int tcb1(void *data) {
    if(!data) {return -1;}
    printf("tcb1: %s\n", (char*)data);
    return 0;
}
static int tcb2(void *data) {
    if(!data) {return -1;}
    printf("tcb2: %s\n", (char*)data);
    return 0;
}

int main(void) {
    taskqueue *tq = taskqueue_init(); task *tsk;
    taskqueue_push(tq, task_init(tcb0, NULL, "data1"));
    taskqueue_pushfront(tq, task_init(tcb1, NULL, "data2"));
    taskqueue_push(tq, task_init(tcb2, NULL, "data3"));
    
    task_fire((tsk = taskqueue_pop(tq)));       task_free(tsk);
    task_fire((tsk = taskqueue_popback(tq)));   task_free(tsk);
    task_fire((tsk = taskqueue_pop(tq)));       task_free(tsk);
    
    taskqueue_push(tq, task_init(tcb0, free, strdup("extra uncalled data")));

    taskqueue_free(tq);

    return 0;
}