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

int taskqueue_handlefirst(taskqueue *tq, task *tsk);

#endif

task * task_init(gcallback callback, fcallback freecb, void *data);
void task_free(void *tsk);
int task_fire(task *tsk);

taskqueue * taskqueue_init(void);                   // Create a FIFO queue of task objects
void taskqueue_free(void *tq);                      // Free a taskqueue
int taskqueue_push(taskqueue *tq, task *tsk);       // Push a task onto the queue
task * taskqueue_pop(taskqueue *tq);                // Pop a task from the queue
int taskqueue_pushfront(taskqueue *tq, task *tsk);  // Push a task to the front of the queue (append, task becomes first in line to be popped)
task * taskqueue_popback(taskqueue *tq);            // Pop a task from the back of the queue (pop the most recently (normally) pushed item)

#endif