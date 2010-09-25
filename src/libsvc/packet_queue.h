#ifndef __PACKET_QUEUE_H_
#define __PACKET_QUEUE_H_

#include "thread.h"
#include "audio.h"

/* This module implements a static queue 
 * This means that no memory allocation is performed during pushs and pops.*/

typedef struct _packet_queue_t {
	unsigned int head;
	unsigned int tail;
	unsigned int size;
	audio_data_t** audio_queue;
	mutex_t queue_mutex;
	mutex_t queue_empty_mutex;
} packet_queue_t;


/* Creates a packet queue with a specified maximum number of packets stored and the packet length. */
packet_queue_t* packet_queue_create(unsigned int queue_size, unsigned int audio_size);

void packet_queue_destroy(packet_queue_t* packet_queue);

/* Pushes an audio packet into a packet queue. 
 * This function doesn't allocate any memory when called.
 * The audio_data gets copied so you need to free it up when not needed. 
 * Returns 0 on success. */
int packet_queue_push_data(packet_queue_t* packet_queue, audio_data_t* audio_data);


/* Copies a packet stored in the head of a packet queue.
 * Please notice that you MUST NOT free the pointer recieved.
 * Calling this function with an empty queue results in pausing the caller thread. */
void packet_queue_pop_data(packet_queue_t* packet_queue, audio_data_t* audio_data);

#endif /* __PACKET_QUEUE_H */
