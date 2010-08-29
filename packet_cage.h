#ifndef __PACKET_CAGE_H_
#define __PACKET_CAGE_H_

#include "thread.h"

#define MAX_TIME 255
typedef unsigned char packet_time_t;

typedef struct _packet_cage_t {
	packet_time_t packet_time;
	audio_packet_t* audio_packet;
	mutex_t cage_mutex;
} packet_cage_t;

packet_time_t time_inc(packet_time_t time);

int is_newer(packet_time_t time1, packet_time_t time2);

packet_cage_t* packet_cage_create();

int packet_cage_destroy(packet_cage_t* packet_cage);

/* Puts an audio packet with a specified time into a packet cage.
 * After calling this function all memory management of audio_packet is left 
 * to the packet cage. It is unsafe to access the audio_packet after that. */
int packet_cage_put_packet(packet_cage_t* packet_cage, audio_packet_t* audio_packet, packet_time_t time);

/* Returns a packet stored in the packet cage making the cage empty or NULL if the cage was already empty.
 * After calling this function the packet cage no longer manages the audio_packet stored in it.
 * The caller is expected to do so. */
audio_packet_t* packet_cage_get_packet(packet_cage_t* packet_cage);

#endif
