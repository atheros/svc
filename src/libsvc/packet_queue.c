#include "packet_queue.h"

packet_queue_t* packet_queue_create(unsigned int queue_size, unsigned int audio_size){
	packet_queue_t* res_queue = malloc(sizeof(packet_queue_t));
	
	mutex_create(&res_queue->queue_mutex);
	mutex_create(&res_queue->queue_empty_mutex);
	mutex_lock(&res_queue->queue_empty_mutex);
	
	res_queue->size = queue_size;
	res_queue->head = 0;
	res_queue->tail = 0;
	
	res_queue->audio_queue = malloc(queue_size*sizeof(audio_data_t*));
	int i;
	for(i=0; i<queue_size; i++)
		res_queue->audio_queue[i] = audio_data_create(audio_size);
	return res_queue;
}

void packet_queue_destroy(packet_queue_t* packet_queue){
	int i;
	for(i=0; i<packet_queue->size; i++)
		audio_data_destroy(packet_queue->audio_queue[i]);
	free(packet_queue->audio_queue);
	mutex_destroy(&packet_queue->queue_mutex);
	mutex_destroy(&packet_queue->queue_empty_mutex);
	
	free(packet_queue);
}

unsigned int next_point_in_queue(unsigned int size, unsigned int pos){
	if (pos == (size-1)) return 0;
	return pos+1;
}

/* !!!
 * This function is not! threadsafe. 
 * !!! */
int packet_queue_is_empty_(packet_queue_t* packet_queue){
	return packet_queue->tail==packet_queue->head;
}

int packet_queue_push_data(packet_queue_t* packet_queue, audio_data_t* audio_data){
	mutex_lock(&packet_queue->queue_mutex);
	
	if (packet_queue_is_empty_(packet_queue)) mutex_unlock(&packet_queue->queue_empty_mutex);
	
	unsigned int new_tail = next_point_in_queue(packet_queue->size, packet_queue->tail);
	if (new_tail == packet_queue->head){ 
		mutex_unlock(&packet_queue->queue_mutex);
		return 1;
	}
	
	packet_queue->tail = new_tail;
	audio_data_copy(packet_queue->audio_queue[new_tail], audio_data);
	
	mutex_unlock(&packet_queue->queue_mutex);
	return 0;
}

void packet_queue_pop_data(packet_queue_t* packet_queue, audio_data_t* audio_data){
	mutex_lock(&packet_queue->queue_empty_mutex);
	mutex_unlock(&packet_queue->queue_empty_mutex);

	mutex_lock(&packet_queue->queue_mutex);

	audio_data_copy(audio_data, packet_queue->audio_queue[packet_queue->head]);
	packet_queue->head = next_point_in_queue(packet_queue->size, packet_queue->head);
	
	if (packet_queue_is_empty_(packet_queue)) mutex_lock(&packet_queue->queue_empty_mutex);
	
	mutex_unlock(&packet_queue->queue_mutex);
}
