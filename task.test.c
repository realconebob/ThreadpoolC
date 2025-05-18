#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int testcb(void *data) {
    if(!data)
        return -1;

    printf("%s\n", (char*)data);
    return -0;
}

int main(void) {
    task *tsk = task_init(testcb, free, strdup("This is some data"));
    task_fire(tsk);
    task_free(tsk);

    return 0;
}