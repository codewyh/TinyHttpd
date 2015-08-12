#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUM 8

typedef struct thread_worker_t_{
	void (*func)(void *);
	void *args;
	struct thread_worker_t_ *next;
} thread_worker_t;

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t cond;

	thread_worker_t *head;
	pthread_t *threads;

	int thread_num;
	int queue_size;
	int shutdown;
	int started;
} tiny_threadpool_t;

/*typedef enum {
	tiny_tp_invalid = -1,
	tiny_tp_lock_fail = -2,
	tiny_tp_already_close = -3,
	tiny_tp_cond_broadcast = -4,
	tiny_tp_thread_fail = -5,

} tiny_threadpool_error_t;*/

tiny_threadpool_t *threadpool_init(int thread_num);

int threadpool_add(tiny_threadpool_t *pool, void (*func)(void *), void *args);

int threadpool_destroy(tiny_threadpool_t *pool, int graceful);
#endif //_THREADPOOL_H_
