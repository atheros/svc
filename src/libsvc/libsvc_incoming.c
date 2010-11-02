#include "libsvc_incoming.h"
#include <stdio.h>

#define MAX_PEERS 1000

peer_t* peer_list;
mutex_t peer_list_mutex;
unsigned int peer_count;

peer_t* if_peer_NULL_unlock(peer_t* peer){
	if(peer==NULL) mutex_unlock(&peer_list_mutex);
	return peer;
}
#define FORALL_PEERS(peer) \
	for(mutex_lock(&peer_list_mutex), peer = peer_list; \
	    if_peer_NULL_unlock(peer)!=NULL; \
	    peer = peer->next)

audio_data_t** mixer_buffer;

audio_data_t* silent_sound;

svc_options_t* svc_options;

void request_incoming_audio(audio_data_t* output_audio_data){
	peer_t* peer;
	int peers_to_mix = 0;
	FORALL_PEERS(peer){
		mixer_buffer[peers_to_mix] = packet_cage_get_data(peer->cage);
		if(mixer_buffer[peers_to_mix]==NULL){ mixer_buffer[peers_to_mix] = silent_sound; printf("no audio from peer\n");}
		peers_to_mix++;
	}
	mix_audio_streams(peers_to_mix, mixer_buffer, output_audio_data);
	int i;
	for(i = 0; i<peers_to_mix; i++){
		if(mixer_buffer[i]!=silent_sound)	
			audio_data_destroy(mixer_buffer[i]);
	}
}

peer_t* svc_peer_join(){
	int xxx;
	peer_t* new_peer = malloc(sizeof(peer_t));
	new_peer->cage = packet_cage_create();
	new_peer->decoder = decoder_create(svc_options->sample_rate, 
	                                   svc_options->frame_size, 
	                                   svc_options->byte_per_packet);

	new_peer->next = peer_list;
	new_peer->prev = NULL;
	mutex_lock(&peer_list_mutex);
	peer_count++;
	if(peer_list==NULL) peer_list = new_peer;
	else{
		peer_list->prev = new_peer;
		peer_list = new_peer;
	}
	mutex_unlock(&peer_list_mutex);
	return new_peer;
}

void svc_peer_leave(peer_t* peer){
	mutex_lock(&peer_list_mutex);
	peer_count--;
	if (peer_list==peer) peer_list = peer->next; 
	if (peer->prev!=NULL) peer->prev->next = peer->next;
	if (peer->next!=NULL) peer->next->prev = peer->prev;
	mutex_unlock(&peer_list_mutex);
	decoder_destroy(peer->decoder);
	packet_cage_destroy(peer->cage);
}

void svc_packet_recieve(network_packet_t* packet, peer_t* peer){
	audio_data_t* audio_data = audio_data_create(svc_options->frame_size);
	decoder_decode(peer->decoder, packet->data, packet->data_len, audio_data->data);
	packet_cage_put_data(peer->cage, audio_data, packet->time);	
}

void incoming_init(svc_options_t* options){
	svc_options = options;
	
	mixer_buffer = malloc(sizeof(audio_data_t*)*MAX_PEERS);
	
	/* Make a silent audio data to fill up lags in network. */
	silent_sound = audio_data_create(svc_options->frame_size);
	clear_audio_data(silent_sound);
	
	mutex_create(&peer_list_mutex);
	peer_count = 0;
	peer_list = NULL;
}

void incoming_close(){
	while(peer_list!=NULL){
		svc_peer_leave(peer_list);
	}
	audio_data_destroy(silent_sound);
	mutex_destroy(&peer_list_mutex);
}
