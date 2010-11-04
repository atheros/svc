#ifndef __THREAD_H
#define __THREAD_H

#ifdef WIN32
#define THREAD_WIN
#else
#define THREAD_POSIX
#endif


#if defined(THREAD_POSIX)
#include <pthread.h>
#elif defined(THREAD_WIN)
#include <windows.h>
#endif


#if defined(THREAD_POSIX)
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
#else
typedef HANDLE thread_t;
typedef HANDLE mutex_t;
typedef HANDLE cond_t;
#endif



int mutex_create(mutex_t* mutex);
int mutex_destroy(mutex_t* mutex);
int mutex_lock(mutex_t* mutex);
int mutex_trylock(mutex_t* mutex);
int mutex_unlock(mutex_t* mutex);



int thread_create(thread_t* thread, void* (*proc)(void*), void* param);
int thread_detach(thread_t thread);
void thread_join(thread_t thread);

/**
 * Exit from current thread.
 */
void thread_exit();

#endif /* __THREAD_H */
