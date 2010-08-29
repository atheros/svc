#include <stdio.h>
#include "thread.h"

#if defined(THREAD_POSIX)
#include <string.h>
#endif


#define MAX_LOOPS 10000
#define MAX_THREADS 10

mutex_t mutex;
int result = 0;

void* worker(void* param) {
	int i;
	for(i = 0; i < MAX_LOOPS; i++) {
		mutex_lock(&mutex);
		result++;
		mutex_unlock(&mutex);
	}
	return NULL;
}


int main(int argc, char* argv[]) {
	int expected = MAX_LOOPS * MAX_THREADS;
	thread_t threads[MAX_THREADS];
	int i, e;
	
	
	for(i = 0; i < MAX_THREADS; i++) {
		e = thread_create(&threads[i], worker, NULL);
		if (e) {
#if defined(THREAD_POSIX)
			fprintf(stderr, "Failed to create thread %i: %s\n", i, strerror(e));
#else
			fprintf(stderr, "Failed to create thread %i\n", i);
#endif
			return 1;
		}
	}
	
	for(i = 0; i < MAX_THREADS; i++) {
		thread_join(threads[i]);
	}
	
	mutex_destroy(&mutex);
	
	printf("Expected: %i\n", expected);
	printf("Got:      %i\n", result);
	
	if (expected != result) {
		fprintf(stderr, "Failure!\n");
		return 1;
	} else {
		printf("Success :)\n");
		return 0;
	}
}



