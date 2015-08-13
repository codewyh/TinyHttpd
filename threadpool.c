#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"
#include "log.h"

typedef enum {
	immediate_shutdown = 1,
	graceful_shutdown = 2,

} threadpool_sd_t;

static void *threadpool_worker(void *args);

tiny_threadpool_t *threadpool_init(int thread_num) {
	if(thread_num <= 0) {
		fprintf(stderr, "threadpool_init :  thread_num error");
		return NULL;
	}
	tiny_threadpool_t *pool;
	pool = (tiny_threadpool_t *)malloc(sizeof(tiny_threadpool_t));
	if(pool == NULL){
		fprintf(stderr,"threadpool_init: pool error");
		return NULL;
	}
	
	pool->thread_num = 0;
	pool->queue_size = 0;
	pool->shutdown = 0;
	pool->started = 0;
	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);

	if(pool->threads == NULL) {
		goto err;
	}

	pool->head = NULL;
	/*pool->head = (thread_worker_t *)malloc(sizeof(thread_worker_t));
	pool->head->func = NULL;
	pool->head->args = NULL;
	pool->head->next = NULL;*/


	if(pthread_mutex_init(&(pool->lock), NULL) != 0) {
		goto err;
	}

	if(pthread_cond_init(&(pool->cond), NULL) != 0) {
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		goto err;
	}

	int i = 0;
	for(;i < thread_num; ++i) {
		if(pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void *)pool) != 0) {
			threadpool_destroy(pool, 0);
			return NULL;
		}
		fprintf(stdout,"thread: %08lux started\n", pool->threads[i]);

		pool->thread_num++;
		pool->started++;
	}

	return pool;

err:
	if(pool) {
		threadpool_free(pool);
	}
	return NULL;
}

int threadpool_add(tiny_threadpool_t *pool, void (*func)(void *), void *args) {
	
	if (pool == NULL || func == NULL) {
		fprintf(stderr, "threadpool_add :  pool or func  NULL");
		return -1;
	}

	thread_worker_t *worker = (thread_worker_t *)malloc(sizeof(thread_worker_t));
	if(worker == NULL){
		return -1;
	}

	worker->func = func;
	worker->args = args;
	worker->next = NULL;

	if(pthread_mutex_lock(&(pool->lock)) != 0) {
		fprintf(stderr, "threadpool_add :  lock error");
		return -1;
	}

	if(pool->shutdown) {
		fprintf(stderr,"threadpool already shutdown");
		return -1;
	}

	thread_worker_t *task = pool->head;
	if(task) {
		while(task->next != NULL)
			task = task->next;
		task->next = worker;
	}
	else {
		pool->head = worker;
	}
	pool->queue_size++;
	
	if(pthread_mutex_unlock(&pool->lock) != 0) {
		fprintf(stderr,"pthread_mutex_unlock");
		return -1;
	}

	if(pthread_cond_signal(&pool->cond) != 0){
		fprintf(stderr,"pthread_cond_signal");
		return -1;
	}
	return 0;			
}

int threadpool_free(tiny_threadpool_t *pool) {
	if(pool == NULL || pool->started > 0) {
		return -1;
	}

	if(pool->threads) {
		free(pool->threads);
	}

	thread_worker_t *tmp;
	while(pool->head){
		tmp = pool->head;
		pool->head = tmp->next;
		free(tmp);
	}
	return 0;
}

int threadpool_destroy(tiny_threadpool_t *pool, int graceful) {
	if(pool == NULL) {
		return -1;
	}

	if (pthread_mutex_lock(&pool->lock) != 0) {
		return -1;
	}

	if (pool->shutdown){
		fprintf(stderr,"threadpool already shutdown");
		return -1;
	}

	pool->shutdown = (graceful)? graceful_shutdown : immediate_shutdown;

	if (pthread_cond_broadcast(&pool->cond) != 0) {
		fprintf(stderr,"cond broadcast err");
		return -1;
	}

	if (pthread_mutex_unlock(&pool->lock) != 0) {
		fprintf(stderr,"mutex unlock err");
		return -1;
	}

	int i = 0;
	for (; i < pool->thread_num; ++i) {
		if (pthread_join(pool->threads[i], NULL) != 0){
			fprintf(stderr, "pthread_join  thread %08lux err",pool->threads[i]);
		}
		printf("thread %08lux exit", pool->threads[i]);
	}
	free(pool->threads);

	thread_worker_t *tmp;
	while(pool->head) {
		tmp = pool->head;
		pool->head = tmp->next;
		free(tmp);
	}

	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->cond);
	//threadpool_free(pool);
	free(pool);

	return 0;
}

static void *threadpool_worker(void *args) {
	if (args == NULL) {
		LOG_INFO("args is NULL");
		return NULL;
	}

	tiny_threadpool_t *pool = (tiny_threadpool_t *)args;
	thread_worker_t *worker;

	while(1) {
		pthread_mutex_lock(&pool->lock);

		while((pool->queue_size == 0) && !(pool->shutdown)) {
			LOG_INFO("thread 0x%lux is waiting",pthread_self());
			pthread_cond_wait(&pool->cond,&pool->lock);
		}

		//LOG_INFO("thread 0x%lus is working", phread_self());
		printf("thread 0x%lux is working\n",pthread_self());

		if(pool->shutdown == immediate_shutdown) {
			//pthread_mutex_unlock(&pool->lock);
			break;
		}
		else if((pool->shutdown == graceful_shutdown) && pool->queue_size == 0) {
			//pthread_mutex_unlock(&pool->lock);
			break;
		}

		worker = pool->head;
		if(worker == NULL)
			continue;

		pool->queue_size--;
		pool->head = worker->next;

		pthread_mutex_unlock(&pool->lock);

		LOG_INFO("begin handle request");
		(*(worker->func))(worker->args);
		LOG_INFO("end handle request");
		free(worker);
		worker = NULL;
	}
	pool->started--;
	pthread_mutex_unlock(&pool->lock);
	pthread_exit(NULL);

	return NULL;
}
