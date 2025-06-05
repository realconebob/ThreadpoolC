#include "../threadpool.h"
#include <stdio.h>
#include <unistd.h>

static int __ctq_testcb(void *data) {
    if(!data) return -1;
    printf("lmao\n");
    return 0;
}

int main() {
    ctqueue *ctq = ctqueue_init(5);
    ctqueue_start(ctq);
    ctqueue_waitpush(ctq, task_init(__ctq_testcb, NULL, "void *data"));

    sleep(1);

    ctqueue_free(ctq);

    return 0;
}

// RAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHH IT WORRRRRRRRRRRRRKKKKKSSSSSSS
// I've been trying to implement this for a personal project for weeks now and now I've finally got it working