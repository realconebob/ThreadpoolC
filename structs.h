#ifndef ___THREADPOOL_IMPLEMENTATION_C__STRUCTS_H__249081690216284___
#define ___THREADPOOL_IMPLEMENTATION_C__STRUCTS_H__249081690216284___ 1

typedef int (*gcallback)(void*);    // Generic callback signature
typedef void (*fcallback)(void*);   // free()-like callback signature

#ifdef ___TPIC__NOT_OPAQUE
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

#include <threads.h>
typedef struct ctqueue {
    mtx_t mutex;
    cnd_t cond;
    unsigned char canceled;

    taskqueue *tq;
    thrd_t *thrdarr;
    int talen;
} ctqueue;
#else

// A task that consists of a generic callback, a free callback, and data to be used as parameters for said callbacks
typedef struct task task;

// A doubly-linked list (queue) of task objects
typedef struct taskqueue taskqueue;

// A concurrent taskqueue
typedef struct ctqueue ctqueue;
#endif

#endif