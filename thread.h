#ifndef __THREAD_H_
#define __THREAD_H_ 

#include <pthread.h>

#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

typedef pthread_mutex_t mutex_t;
typedef pthread_t thread_t;

int thread_create(thread_t *thread, void *(*start_routine)(void*), void *arg){
	return pthread_create(thread, NULL, start_routine, arg);
}

int thread_join(thread_t thread){
	return pthread_join(thread, NULL);
}

int mutex_lock(mutex_t *mutex){
	return pthread_mutex_lock(mutex);
}

int mutex_unlock(mutex_t *mutex){
	return pthread_mutex_unlock(mutex);
}

int mutex_destroy(mutex_t *mutex){
	return pthread_mutex_destroy(mutex);
}


#endif
