#define ___TPIC__NOT_OPAQUE 1
#include "../threadpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int testcb(void *data) {
    if(!data)
        return -1;

    printf("%s\n", (char*)data);
    return 0;
}

int main(void) {
    task *tsk = task_init(testcb, free, strdup("This is some data"));
    tqnode *tqn = tqnode_init(NULL, NULL, tsk);
    tqnode_free(tqn);
    
    return 0;
}