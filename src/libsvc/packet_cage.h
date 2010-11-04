#ifndef __PACKET_CAGE_H_
#define __PACKET_CAGE_H_

#include "thread.h"
#include "audio.h"

#define MAX_TIME 65535
typedef unsigned int packet_time_t;

typedef struct _packet_cage_t {
	unsigned int head;
	unsigned int size;
	packet_time_t head_time;
	packet_time_t tail_time;
	int cage_starvation;
	int new_cage;
	audio_data_t** audio_queue;
	mutex_t cage_mutex;
} packet_cage_t;

packet_time_t time_inc(packet_time_t time);

int is_newer(packet_time_t time1, packet_time_t time2);

int sub_time(packet_time_t time1, packet_time_t time2){


packet_cage_t* packet_cage_create(unsigned int size);

int packet_cage_destroy(packet_cage_t* packet_cage);

/* Puts an audio packet with a specified time into a packet cage.
 * After calling this function all memory management of audio_packet is left 
 * to the packet cage. It is unsafe to access the audio_packet after that. */
int packet_cage_put_data(packet_cage_t* packet_cage, audio_data_t* audio_data, packet_time_t time);

/* Returns a packet stored in the packet cage making the cage empty or NULL if the cage was already empty.
 * After calling this function the packet cage no longer manages the audio_data stored in it.
 * The caller is expected to do so. */
audio_data_t* packet_cage_get_data(packet_cage_t* packet_cage);

#endif /* __PACKET_CAGE_H */
