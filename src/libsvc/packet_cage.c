#include "packet_cage.h"
#include <stdlib.h>

#include <stdio.h>

packet_time_t time_inc(packet_time_t time){
	if(time==MAX_TIME) return 0;
	return time+1;
}

packet_time_t time_inc_by(packet_time_t time, int delta){
	return (time+delta)%(MAX_TIME+1);
}

int is_newer(packet_time_t time1, packet_time_t time2){
	int delta = (int)time1 - (int)time2;
	
	return ((delta>0) && (delta< (MAX_TIME+1)/2)) ||
	       ((delta<0) && (delta<-(MAX_TIME+1)/2));
}

int sub_time(packet_time_t time1, packet_time_t time2){
	int delta = (int)time1 - (int)time2;
	
	if(is_newer(time1, time2)) return (delta + (MAX_TIME+1))%(MAX_TIME+1);
	return -(-delta + (MAX_TIME+1))%(MAX_TIME+1);
	
}

packet_cage_t* packet_cage_create(unsigned int queue_size){
	packet_cage_t* res_packet_cage = malloc(sizeof(packet_cage_t));
	
	
	res_packet_cage->size = queue_size;
	res_packet_cage->head = 0;
	res_packet_cage->new_cage = 1;
	
	res_packet_cage->audio_queue = malloc(queue_size*sizeof(audio_data_t*));
	int i;
	for(i=0; i<queue_size; i++) 
		res_packet_cage->audio_queue[i] = NULL;
	
	mutex_create(&res_packet_cage->cage_mutex);
	res_packet_cage->head_time = 0;
	res_packet_cage->tail_time = 0;
	res_packet_cage->cage_starvation = 1;
	
	return res_packet_cage;
}


int packet_cage_destroy(packet_cage_t* packet_cage){
	int i;
	for(i=0; i<packet_cage->size; i++)
		if(packet_cage->audio_queue[i]!=NULL) 
			free(packet_cage->audio_queue[i]);
	
	free(packet_cage->audio_queue);
	
	mutex_destroy(&packet_cage->cage_mutex);
	
	free(packet_cage);
	return 0;
}

static unsigned int next_point_in_cage(unsigned int size, unsigned int pos){
	if (pos == (size-1)) return 0;
	return pos+1;
}

/* FIXME: Is is used? */
static unsigned int delta_point_in_cage(unsigned int size, unsigned int pos, int delta){
	return ((pos+delta)%size + size)%size;
}

/* !!!
 * This function is not! threadsafe. 
 * !!! */
int packet_cage_is_empty(packet_cage_t* packet_cage){
	return packet_cage->tail_time==packet_cage->head_time;
}


void packet_cage_pop(packet_cage_t* packet_cage){
	if (packet_cage->audio_queue[packet_cage->head] != NULL) {
		audio_data_destroy(packet_cage->audio_queue[packet_cage->head]);
	}
	
	packet_cage->audio_queue[packet_cage->head] = NULL;
	packet_cage->head = next_point_in_cage(packet_cage->size, packet_cage->head);
	packet_cage->head_time = time_inc(packet_cage->head_time);

	if(packet_cage_is_empty(packet_cage)) {
		packet_cage->cage_starvation = 1;
		packet_cage->new_cage = 1;
	}
}

int packet_cage_put_data(packet_cage_t* packet_cage, audio_data_t* audio_data, packet_time_t time){
	mutex_lock(&packet_cage->cage_mutex);
	
	/* We only place the packet when it is newer then the one expected or we have no packets in the queue. */
	if (packet_cage->new_cage||is_newer(time, packet_cage->head_time)){
		if(packet_cage->new_cage){
			packet_cage->head_time = time;
			packet_cage->new_cage = 0;
		}
		/* If by some chance we have some realy old packets in the cage, we must drop them. 
		 * This also signals us that the cage is full.*/
		while(is_newer(time, time_inc_by(packet_cage->head_time, packet_cage->size-1))){
			packet_cage_pop(packet_cage);
			packet_cage->cage_starvation = 0;
			printf("poped useless stuff\n");
		}
		/* Calculating where to write the new packet. */
		int new_elem_pos = delta_point_in_cage(packet_cage->size, 
		                                       packet_cage->head, 
		                                       sub_time(time, packet_cage->head_time));
		packet_cage->audio_queue[new_elem_pos] = audio_data;
		if(is_newer(time, packet_cage->tail_time)) packet_cage->tail_time = time;
	} else printf("a realy old packet arived\n");
	
	mutex_unlock(&packet_cage->cage_mutex);
	return 0;
}

audio_data_t* packet_cage_get_data(packet_cage_t* packet_cage){
	mutex_lock(&packet_cage->cage_mutex);
	audio_data_t* res_audio_data;
	if(!packet_cage->cage_starvation){
		res_audio_data = packet_cage->audio_queue[packet_cage->head];
		packet_cage->audio_queue[packet_cage->head] = NULL;
		
		packet_cage->head = next_point_in_cage(packet_cage->size, packet_cage->head);
		packet_cage->head_time = time_inc(packet_cage->head_time);
		
		if(packet_cage_is_empty(packet_cage)){ 
			packet_cage->cage_starvation = 1;
			packet_cage->new_cage = 1;
		}
		
	}else{
		printf("starving...\n");
		res_audio_data = NULL;
	}
	
	mutex_unlock(&packet_cage->cage_mutex);
	
	return res_audio_data;
}
