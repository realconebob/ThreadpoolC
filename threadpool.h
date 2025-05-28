#ifndef ___THREADPOOL_IMPLEMENTATION_C__192741377429633___
#define ___THREADPOOL_IMPLEMENTATION_C__192741377429633___ 1

typedef int (*gcallback)(void*);    // Generic callback signature
typedef void (*fcallback)(void*);   // free()-like callback signature

#ifndef ___TPIC__NOT_OPAQUE
// Testing opaque data structs

// A task that consists of a generic callback, a free callback, and data to be used as parameters for said callbacks
typedef struct task task;

// A doubly-linked list (queue) of task objects
typedef struct taskqueue taskqueue;

// A concurrent taskqueue
typedef struct ctqueue ctqueue;

// A locally defined structure designed for easier function cleanup
typedef struct cl cleanup;

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

#include <threads.h>
typedef struct ctqueue {
    mtx_t mutex;
    cnd_t cond;
    unsigned char canceled;

    taskqueue *tq;
    thrd_t *thrdarr;
    int tasize;
} ctqueue;

typedef struct cl {
    fcallback *callbacks;   // Actual Type: fcallback callbacks[]
    void * *arguments;      // Actual Type: void *arguments[]
    int size;
    int used;
} cleanup;

tqnode * tqnode_init(tqnode *next, tqnode *prev, task *tsk);
void tqnode_free(void *tqn);

int taskqueue_handlefirst(taskqueue *tq, task *tsk);

#endif

int cleanup_init(cleanup *loc, fcallback callbacks[], void *arguments[], int size);
int cleanup_register(cleanup *loc, fcallback cb, void *arg);
int cleanup_cndregister(cleanup *loc, fcallback cb, void *arg, unsigned char flag);
int cleanup_clear(cleanup *loc);
int cleanup_fire(cleanup *loc);
int cleanup_cndfire(cleanup *loc, unsigned char flag);

#define cleanup_CREATE(size) \
cleanup __CLEANUP; \
fcallback __CLEANUP_FUNCS[(size)]; \
void *__CLEANUP_ARGS[(size)]; \
unsigned char __FLAG = 0; \
cleanup_init(&__CLEANUP, __CLEANUP_FUNCS, __CLEANUP_ARGS, (size))

#define cleanup_REGISTER(cb, arg)       cleanup_register(&__CLEANUP, (cb), (arg))
#define cleanup_CNDREGISTER(cb, arg)    cleanup_cndregister(&__CLEANUP, (cb), (arg), __FLAG)
#define cleanup_CLEAR()                 cleanup_clear(&__CLEANUP)
#define cleanup_FIRE()                  cleanup_fire(&__CLEANUP)
#define cleanup_CNDFIRE()               cleanup_cndfire(&__CLEANUP, __FLAG)
#define cleanup_MARK()                  (__FLAG = 1)
#define cleanup_UNMARK()                (__FLAG = 0)
#define cleanup_ERRORFLAGGED            (__FLAG != 0)
#define cleanup_CNDEXEC(code)           while(!cleanup_ERRORFLAGGED) {code; break;}

// Create a new task. Sets `errno` and returns `NULL` on error
task * task_init(gcallback callback, fcallback freecb, void *data);

// Free a task. Passes the `.data` member to specified fcallback as a function parameter. Does not return a value or set `errno`
void task_free(void *tsk);

// Fire a task. Passes the `.data` member to specified gcallback as a function parameter. Returns value of gcallback, or sets `errno` and returns `-1` on error
int task_fire(task *tsk);

// Fire and free a task simultaneously. Calls specified gcallback and fcallback on associated data. Returns value of gcallback, or sets `errno` and returns `-1` on error
int task_fired(task *tsk);                                              



// Create a FIFO queue of task objects. Returns a new taskqueue on success, sets `errno` and returns `NULL` on error
taskqueue * taskqueue_init(void);

// Free a taskqueue. Does not return a value or set `errno`
void taskqueue_free(void *tq);

// Push a task onto the queue. Returns 0 on success, sets `errno` and returns `-1` on error
int taskqueue_push(taskqueue *tq, task *tsk);

// Pop a task from the queue. Returns a task on success, sets `errno` and returns `NULL` on error
task * taskqueue_pop(taskqueue *tq);

// Push a task to the front of the queue (append, task becomes first in line to be popped). Returns 0 on success, sets `errno` and returns `-1` on error
int taskqueue_pushfront(taskqueue *tq, task *tsk);

// Pop a task from the back of the queue (pop the most recently (normally) pushed item). Returns a task on success, sets `errno` and returns `NULL` on error
task * taskqueue_popback(taskqueue *tq);

#endif