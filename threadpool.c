#include "threadpool.h"

#include <stdlib.h>
#include <errno.h>
#include <error.h>

#ifndef ___TPIC__NOT_OPAQUE
typedef struct task {
    gcallback callback;
    fcallback freecb;
    void *data;
} task;

typedef struct tqnode {
    struct tqnode *next;
    struct tqnode *prev;
    task *task;
} tqnode;

typedef struct taskqueue {
    tqnode *start;
    tqnode *end;
    unsigned int size;
} taskqueue;
#endif

task * task_init(gcallback callback, fcallback freecb, void *data) {
    if(callback == NULL) {errno = EINVAL; return NULL;}

    task *tsk = calloc(1, sizeof(*tsk));
    if(!tsk)
        return NULL;

    tsk->callback = callback;
    tsk->freecb = freecb;
    tsk->data = data;

    return tsk;
}

void task_free(void *tsk) {
    task *real = (task *)tsk;
    if(!real)
        return;
    
    if(real->freecb != NULL)
        real->freecb(real->data);
    free(real);

    return;
}

int task_fire(task *tsk) {
    if(!tsk) {errno = EINVAL; return -1;}
    return tsk->callback(tsk->data);
}



tqnode * tqnode_init(tqnode *next, tqnode *prev, task *tsk) {
    if(!tsk) {errno = EINVAL; return NULL;}
    tqnode *node = calloc(1, sizeof(*node));
    if(!node)
        return NULL;

    node->next = next;
    node->prev = prev;
    node->task = tsk;

    return node;
}

void tqnode_free(void *tqn) {
    tqnode *real = (tqnode *)tqn;
    if(!real)
        return;

    task_free(real->task);
    free(real);
    return;
}

// Not bothering with a set/get next/prev because not only is this a monolithic file for the sake of ease, but even in a real implementation the node and actual dqueue structure would be in the same file anyway



taskqueue * taskqueue_init(void) {
    taskqueue *tq = calloc(1, sizeof(*tq));
    if(!tq)
        return NULL;

    tq->start = NULL;
    tq->end = NULL;
    tq->size = 0;

    return tq;
}

void taskqueue_free(void *tq) {
    if(!tq)
        return;

    for(tqnode *p = ((taskqueue*)tq)->start, *n; p != NULL;) {
        n = p->next;
        tqnode_free(p);
        p = n;
    }
    free(tq);
 
    return;
}

// push pop nomenclature, but slightly modified:
    // Normal:
        // push - push to front of queue
        // pop  - pop from end of queue
    // Modified:
        // pushback  - push to end of queue
        // popfront - pop from front of queue

// I could borrow the java names (iirc one of them was "offer") but I don't like java so too bad

int taskqueue_handlefirst(taskqueue *tq, task *tsk) {
    if(!tq || !tsk) {errno = EINVAL; return -1;}
    if(tq->size) {return 0;}

    tqnode *first = tqnode_init(NULL, NULL, tsk);
    if(!first)
        return -1;

    tq->start = first;
    tq->end = first;
    tq->size = 1;

    return 1;
}

int taskqueue_push(taskqueue *tq, task *tsk) {
    if(!tq || !tsk) {errno = EINVAL; return -1;}

    int hf;
    if((hf = taskqueue_handlefirst(tq, tsk)))
        return (hf >= 0) ? 0 : -1;

    tqnode *newstart = tqnode_init(tq->start, NULL, tsk);
    if(!newstart)
        return -1;
    tq->start->prev = newstart;
    tq->start = newstart;
    tq->size++;

    return 0;
}

task * taskqueue_pop(taskqueue *tq) {
    if(!tq) {errno = EINVAL; return NULL;}
    if(tq->size <= 0) {errno = ENODATA; return NULL;}

    tqnode *end = tq->end;
    task *ret = end->task; 

    if(tq->size == 1) {
        tq->end = NULL;
        tq->start = NULL;
    } else {
        tq->end = end->prev;
        tq->end->next = NULL;
    }

    free(end);
    tq->size--;
    return ret;
}

int taskqueue_pushfront(taskqueue *tq, task *tsk) {
    if(!tq || !tsk) {errno = EINVAL; return -1;}

    int hf;
    if((hf = taskqueue_handlefirst(tq, tsk)))
        return (hf >= 0) ? 0 : -1;

    tqnode *newend = tqnode_init(NULL, tq->end, tsk);
    if(!newend)
        return -1;
    tq->end->next = newend;
    tq->end = newend;
    tq->size++;

    return 0;
}

task * taskqueue_popback(taskqueue *tq) {
    if(!tq) {errno = EINVAL; return NULL;}
    if(tq->size <= 0) {errno = ENODATA; return NULL;}

    tqnode *start = tq->start;
    task *ret = start->task;

    if(tq->size == 1) {
        tq->start = NULL;
        tq->end = NULL;
    } else {
        tq->start = start->next;
        tq->start->prev = NULL;
    }

    free(start);
    tq->size--;
    return ret;
}

// TODO: Test the taskqueuenode to make sure it doesn't leak memory somehow