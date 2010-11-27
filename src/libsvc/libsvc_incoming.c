#include "libsvc_incoming.h"
#include <stdio.h>

#define MAX_PEERS 1000

static svc_peer_t* peer_list;
static mutex_t peer_list_mutex;
static unsigned int peer_count;

static svc_peer_t* if_peer_NULL_unlock(svc_peer_t* peer){
	if(peer==NULL) mutex_unlock(&peer_list_mutex);
	return peer;
}
#define FORALL_PEERS(peer) \
	for(mutex_lock(&peer_list_mutex), peer = peer_list; \
	    if_peer_NULL_unlock(peer)!=NULL; \
	    peer = peer->next)

static svc_audio_data_t** mixer_buffer;

static svc_audio_data_t* silent_sound;

static svc_options_t* svc_options;

void svc_request_incoming_audio(svc_audio_data_t* output_audio_data){
	svc_peer_t* peer;
	int peers_to_mix = 0;
	FORALL_PEERS(peer){
		mixer_buffer[peers_to_mix] = packet_cage_get_data(peer->cage);
		if(mixer_buffer[peers_to_mix]==NULL){ mixer_buffer[peers_to_mix] = silent_sound; printf("no audio from peer\n"); }
		peers_to_mix++;
	}
	svc_mix_audio_streams(peers_to_mix, mixer_buffer, output_audio_data);
	int i;
	for(i = 0; i<peers_to_mix; i++){
		if(mixer_buffer[i]!=silent_sound)	
			svc_audio_data_destroy(mixer_buffer[i]);
	}
}

svc_peer_t* svc_peer_join(){
	svc_peer_t* new_peer = malloc(sizeof(svc_peer_t));
	new_peer->cage = packet_cage_create(5);
	new_peer->decoder = svc_decoder_create(svc_options->sample_rate, 
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

void svc_peer_leave(svc_peer_t* peer){
	mutex_lock(&peer_list_mutex);
	peer_count--;
	if (peer_list==peer) peer_list = peer->next; 
	if (peer->prev!=NULL) peer->prev->next = peer->next;
	if (peer->next!=NULL) peer->next->prev = peer->prev;
	mutex_unlock(&peer_list_mutex);
	svc_decoder_destroy(peer->decoder);
	packet_cage_destroy(peer->cage);
}

void svc_packet_recieve(network_packet_t* packet, svc_peer_t* peer){
	svc_audio_data_t* audio_data = svc_audio_data_create(svc_options->frame_size);
	svc_decoder_decode(peer->decoder, packet->data, packet->data_len, audio_data->data);
	packet_cage_put_data(peer->cage, audio_data, packet->time);	
}

void svc_incoming_init(svc_options_t* options){
	svc_options = options;
	
	mixer_buffer = malloc(sizeof(svc_audio_data_t*)*MAX_PEERS);
	
	/* Make a silent audio data to fill up lags in network. */
	silent_sound = svc_audio_data_create(svc_options->frame_size);
	svc_clear_audio_data(silent_sound);
	
	mutex_create(&peer_list_mutex);
	peer_count = 0;
	peer_list = NULL;
}

void svc_incoming_close(){
	while(peer_list!=NULL){
		svc_peer_leave(peer_list);
	}
	svc_audio_data_destroy(silent_sound);
	mutex_destroy(&peer_list_mutex);
}
