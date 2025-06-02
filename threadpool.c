#define ___TPIC__NOT_OPAQUE
#include "structs.h"
#undef ___TPIC__NOT_OPAQUE
#include "threadpool.h"

#include <threads.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>



int cleanup_init(cleanup *loc, fcallback callbacks[], void *arguments[], int size) {
    if(!loc || !callbacks || !arguments || size <= 0) {errno = EINVAL; return -1;}

    loc->callbacks = callbacks;
    loc->arguments = arguments;
    loc->size = size;
    loc->used = 0;

    return 0;
}

// registers if flag is NOT set
int cleanup_register(cleanup *loc, fcallback cb, void *arg) {
    if(!loc || !cb) {errno = EINVAL; return -1;}
    if(loc->used >= loc->size || loc->used < 0) {errno = ENOMEM; return -1;}

    loc->callbacks[loc->used] = cb;
    loc->arguments[loc->used] = arg;
    loc->used++;

    return 0;
}

int cleanup_cndregister(cleanup *loc, fcallback cb, void *arg, unsigned char flag) {
    if(flag)
        return 0;
    return cleanup_register(loc, cb, arg);
}

int cleanup_clear(cleanup *loc) {
    if(!loc) {errno = EINVAL; return -1;}
    
    loc->used = 0;
    return 0;
}
/* I know very well that this isn't "clearing", but that's fine, as I basically never intend to clear the stack, and assume that 
// you'd keep a reference instead of pawning it off on this (which would be dumb because of the lack of retrieval functions) */

int cleanup_fire(cleanup *loc) {
    if(!loc) {errno = EINVAL; return -1;}

    for(int i = (loc->used - 1); i >= 0; i--) {
        if(loc->callbacks[i] == NULL) {
            error(0, EINVAL, "cleanup_fire: refusing to run null callback...");
            continue;
        }

        loc->callbacks[i](loc->arguments[i]);
    }
    cleanup_clear(loc);

    return 0;
}

// Fires if flag is set
int cleanup_cndfire(cleanup *loc, unsigned char flag) {
    if(flag)
        return cleanup_fire(loc);
    return 0;
}



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

int task_fired(task *tsk) {
    int retval = task_fire(tsk);
    if(errno == EINVAL && retval == -1) {return -1;}
    task_free(tsk);
    return retval;
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
        // push - push to back of queue
        // pop  - pop from front of queue
    // Modified:
        // pushfront  - push to front of queue
        // popback - pop from back of queue

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

int taskqueue_size(taskqueue *tq) {
    if(!tq) {errno = EINVAL; return -1;}
    return tq->size;
}

// TODO: Consolidate push/pop functions into a single push and pop function respectively, then expose push/pop front/back behavior with helper functions



// Internal helper macro for ctq functions. Acquires a lock via the ctq's mutex, checks to see if the queue has been canceled, then executes "code" as written
#define __CTQ_INLOCK(ctq, retval, code) do {\
    mtx_lock(&(ctq)->mutex); \
    if((ctq)->canceled) { \
        errno = ECANCELED; \
        mtx_unlock(&(ctq)->mutex); \
        return (retval); \
    } \
    \
    code \
    mtx_unlock(&(ctq)->mutex); \
} while (0)

static void ___ucl_mtxdestroy(void *mtx) {
    if(!mtx) return;
    mtx_destroy((mtx_t *)mtx);
    return;
}

static void ___ucl_cnddestroy(void *cond) {
    if(cond) return;
    cnd_destroy((cnd_t *)cond);
    return;
}

ctqueue * ctqueue_init(int nthreads) {
    if(nthreads <= 0) {errno = EINVAL; return NULL;}
    cleanup_CREATE(6);

    ctqueue *ctq = calloc(1, sizeof(*ctq));
    if(!ctq)
        return NULL;
    cleanup_REGISTER(free, ctq);

    ctq->canceled = 0;
    ctq->talen = nthreads;

    cleanup_CNDEXEC(
        ctq->tq = taskqueue_init();
        if(!ctq->tq)
            cleanup_MARK();
        cleanup_CNDREGISTER(taskqueue_free, ctq->tq);
    );
    
    cleanup_CNDEXEC(
        if(mtx_init(&ctq->mutex, mtx_plain) != thrd_success)
            cleanup_MARK();
        cleanup_CNDREGISTER(___ucl_mtxdestroy, (void*)&ctq->mutex);
    );

    cleanup_CNDEXEC(
        if(cnd_init(&ctq->cond) != thrd_success)
            cleanup_MARK();
        cleanup_CNDREGISTER(___ucl_cnddestroy, (void*)&ctq->cond);
    );
    
    cleanup_CNDEXEC(
        ctq->thrdarr = calloc(ctq->talen, sizeof(thrd_t));
        if(!ctq->thrdarr)
            cleanup_MARK();
        cleanup_CNDREGISTER(free, ctq->thrdarr);
    )

    cleanup_CNDFIRE();
    if(cleanup_ERRORFLAGGED)
        return NULL;

    return ctq;
}

int ctqueue_cancel(ctqueue *ctq) {
    if(!ctq) {errno = EINVAL; return -1;}

    __CTQ_INLOCK(ctq, 1, 
        ctq->canceled = 1;
    );
    cnd_broadcast(&ctq->cond);

    return 0;
}

void ctqueue_free(void *ctq) {
    if(!ctq)
        return;

    ctqueue *real = (ctqueue *)ctq;
    ctqueue_cancel(real);

    // Not sure if I want to / should block on the mutex or not. I believe it would cause a deadlock if I did (consumer waiting on mutex, free waiting on consumer to quit)
    for(int i = 0; i < real->talen; i++)
        thrd_join(real->thrdarr[i], NULL);

    // Threads are dead, everything's free game
    mtx_destroy(&real->mutex);
    cnd_destroy(&real->cond);
    taskqueue_free(real->tq);
    free(real->thrdarr);
    free(real);

    // TODO: figure out how to do error handling for each individual data member, if necessary

    return;
}

int ctqueue_waitpush(ctqueue *ctq, task *tsk) {
    if(!ctq || !tsk) {errno = EINVAL; return -1;}
    int retval = 0;

    __CTQ_INLOCK(ctq, -1, 
        retval = taskqueue_push(ctq->tq, tsk);
    );
    if(retval == 0)
        cnd_signal(&ctq->cond);

    return retval;
}

task * ctqueue_waitpop(ctqueue *ctq) {
    if(!ctq) {errno = EINVAL; return NULL;}
    task *retval = NULL;

    __CTQ_INLOCK(ctq, NULL, 
        while(taskqueue_size(ctq->tq) == 0 && !ctq->canceled)
            cnd_wait(&ctq->cond, &ctq->mutex);

        if(ctq->canceled) {
            errno = ECANCELED;
            mtx_unlock(&ctq->mutex);
            return NULL;
        }

        retval = taskqueue_pop(ctq->tq);
    );

    return retval;
}

static int __CTQ_CONSUMER(void *ctq) {
    if(!ctq) {errno = EINVAL; thrd_exit(-1);}
    ctqueue *real = (ctqueue *)ctq; 

    for(task *ctask = NULL;;) {
        ctask = ctqueue_waitpop(real);
        if(!ctask)
            break;

        task_fire(ctask);
    }

    thrd_exit(1); // non-zero indicates error, -1 indicates invalid argument
}

int ctqueue_start(ctqueue *ctq) {
    if(!ctq) {errno = EINVAL; return -1;}

    ctq->canceled = 0;
    
    int retval = 0;
    for(int i = 0; i < ctq->talen; i++)
        if((retval = thrd_create(&ctq->thrdarr[i], __CTQ_CONSUMER, ctq)) != thrd_success)
            break;

    if(retval != thrd_success)
        ctqueue_cancel(ctq);

    return (retval == thrd_success) ? 0 : -1;
}