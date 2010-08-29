#include "packet_cage.h"
#include <stdlib.h>

packet_time_t time_inc(packet_time_t time){
	if(time==MAX_TIME) return 0;
	return time+1;
}

int is_newer(packet_time_t time1, packet_time_t time2){
	int delta = (int)time1 - (int)time2;
	
	return ((delta>0) && (delta< MAX_TIME/2)) ||
	       ((delta<0) && (delta<-MAX_TIME/2));
}

packet_cage_t* packet_cage_create(){
	packet_cage_t* res_packet_cage = malloc(sizeof(packet_cage_t));
	
	res_packet_cage->audio_packet = NULL;
	mutex_create(res_packet_cage->cage_mutex);
	res_packet_cage->packet_order = 0;
	
	return res_packet_cage;
}

int packet_cage_destroy(packet_cage_t* packet_cage){
	
	if(packet_cage->audio_packet!=NULL) 
		free(packet_cage->audio_packet);
	
	mutex_destroy(packet_cage->cage_mutex);
	
	free(packet_cage);
	return 0;
}

int packet_cage_put_packet(packet_cage_t* packet_cage, audio_packet_t* audio_packet, packet_time_t time){
	mutex_lock(packet_cage->cage_mutex);
	
	/*replace a packet if there is a newer one or the cage is empty*/
	if (is_newer(time, packet_cage->packet_time) || packet_cage->audio_packet==NULL){ 
		if(packet_cage->audio_packet!=NULL) 
			free(packet_cage->audio_packet);
		
		packet_cage->audio_packet = audio_packet;
		packet_cage->packet_time = time;
	}
	
	mutex_unlock(packet_cage->cage_mutex);
	return 0;
}


audio_packet_t* packet_cage_get_packet(packet_cage_t* packet_cage){
	mutex_lock(packet_cage->cage_mutex);
	
	audio_packet_t* res_audio_packet = packet_cage->audio_packet;
	packet_cage->audio_packet = NULL;
	
	mutex_unlock(packet_cage->cage_mutex);
	
	return res_audio_packet;
}
