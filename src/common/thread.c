#include "thread.h"



int mutex_create(mutex_t* mutex) {
#if defined(THREAD_POSIX)
	return pthread_mutex_init(mutex, NULL);
#elif defined(THREAD_WIN)
	*mutex = CreateMutex(NULL, FALSE, NULL);
	if (*mutex == NULL) {
		return -1;
	} else {
		return 0;
	}
#endif
}

int mutex_destroy(mutex_t* mutex) {
#if defined(THREAD_POSIX)
	return pthread_mutex_destroy(mutex);
#elif defined(THREAD_WIN)
	if (CloseHandle(*mutex)) {
		return 0;
	} else {
		return -1;
	}
#endif
}

int mutex_lock(mutex_t* mutex) {
#if defined(THREAD_POSIX)
	return pthread_mutex_lock(mutex);
#elif defined(THREAD_WIN)
	if (WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED) {
		return -1;
	} else {
		return 0;
	}
#endif
}

int mutex_trylock(mutex_t* mutex) {
#if defined(THREAD_POSIX)
	return pthread_mutex_trylock(mutex);
#elif defined(THREAD_WIN)
	if (WaitForSingleObject(*mutex, 0) != WAIT_OBJECT_0) {
		return -1;
	} else {
		return 0;
	}
#endif
}

int mutex_unlock(mutex_t* mutex) {
#if defined(THREAD_POSIX)
	return pthread_mutex_unlock(mutex);
#elif defined(THREAD_WIN)
	if (ReleaseMutex(*mutex)) {
		return 0;
	} else {
		return -1;
	}
#endif
}


#if defined(THREAD_WIN)

struct RunData {
	void* (*proc)(void*);
	void* param;
}

static int run_thread(void* data) {
	struct RunData rd = *(struct RunData*)data;
	free(data);
	(rd.proc)(rd.param);
	return 0;
}
#endif


int thread_create(thread_t* thread, void* (*proc)(void*), void* param) {
#if defined(THREAD_POSIX)
	return pthread_create(thread, NULL, proc, param);
#elif defined(THREAD_WIN)
	struct RunData* rd = (struct RunData*)malloc(sizeof(struct RunData));
	rd->proc = proc;
	rd->param = param;
	
	*thread = CreateThread(NULL, 0, run_thread, rd, NULL);
	if (*thread == NULL) {
		return -1;
	} else {
		return 0;
	}
#endif
}

int thread_detach(thread_t thread){
#if defined(THREAD_POSIX)
	return pthread_detach(thread);
#elif defined(THREAD_WIN)
 /*  TODO  */
#error Not implemented 
#endif	

}

void thread_join(thread_t thread) {
#if defined(THREAD_POSIX)
	pthread_join(thread, 0);
#elif defined(THREAD_WIN)
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
#endif
}

void thread_exit() {
#if defined(THREAD_POSIX)
	pthread_exit(NULL);
#elif defined(THREAD_WIN)
	ExitThread(0);
#endif
}

