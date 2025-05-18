#ifndef ___THREADPOOL_IMPLEMENTATION_C__192741377429633___
#define ___THREADPOOL_IMPLEMENTATION_C__192741377429633___ 1

typedef int (*gcallback)(void*);    // Generic callback signature
typedef void (*fcallback)(void*);   // free()-like callback signature

#ifndef ___TPIC__NOT_OPAQUE
// Testing opaque data structs
typedef struct task task;
typedef struct taskqueue taskqueue;

#else
// For when testing requires knowledge of the structure's insides
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

tqnode * tqnode_init(tqnode *next, tqnode *prev, task *tsk);
void tqnode_free(void *tqn);

#endif

task * task_init(gcallback callback, fcallback freecb, void *data);
void task_free(void *tsk);
int task_fire(task *tsk);

#endif