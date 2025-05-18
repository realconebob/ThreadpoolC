#ifndef ___THREADPOOL_IMPLEMENTATION_C__192741377429633___
#define ___THREADPOOL_IMPLEMENTATION_C__192741377429633___ 1

typedef int (*gcallback)(void*);    // Generic callback signature
typedef void (*fcallback)(void*);   // free()-like callback signature
typedef struct task task;

task * task_init(gcallback callback, fcallback freecb, void *data);
void task_free(void *tsk);
int task_fire(task *tsk);

#endif