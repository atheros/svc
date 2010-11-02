#include "libsvc.h"
#include "packet_cage.h"
#include "packet_queue.h"
#include "thread.h"
#include "audio.h"
#include <stdlib.h>
#include <string.h>

svc_options_t* svc_options;

packet_time_t svc_time;

svc_send_callback_t svc_send_callback;

packet_queue_t* packet_queue;

encoder_t* svc_encoder;

thread_t svc_send_thread;

void audio_api_callback(audio_data_t* input_audio_data, audio_data_t* output_audio_data){
	packet_queue_push_data(packet_queue, input_audio_data);
	request_incoming_audio(output_audio_data);		
}

network_packet_t* create_network_packet_from_audio(audio_data_t* audio_data){
	network_packet_t* network_packet = malloc(sizeof(network_packet_t));
	network_packet->data = malloc(sizeof(unsigned char)*svc_options->byte_per_packet);
	network_packet->data_len = svc_options->byte_per_packet;
	encoder_encode(svc_encoder, audio_data->data, network_packet->data);
	svc_time = time_inc(svc_time);
	network_packet->time = svc_time;
	return network_packet;
}

void *send_network_thread_function( void *ptr ){
	audio_data_t* audio_data = audio_data_create(svc_options->frame_size);
	while(1){
		packet_queue_pop_data(packet_queue, audio_data);
		network_packet_t* network_packet = create_network_packet_from_audio(audio_data);
		svc_send_callback(network_packet);
		network_packet_destroy(network_packet);
	}
}


void svc_init(svc_send_callback_t send_callback){
	svc_time = 0;
	svc_options = malloc(sizeof(svc_options_t));
	svc_options->frame_size = 512;
	svc_options->sample_rate = 44100;
	svc_options->byte_per_packet = 2560;
	
	svc_encoder = encoder_create(svc_options->sample_rate, svc_options->frame_size, svc_options->byte_per_packet);
	packet_queue = packet_queue_create(100, svc_options->frame_size);
	
	incoming_init(svc_options);
	
	svc_send_callback = send_callback;		
	
	thread_create(&svc_send_thread, send_network_thread_function, NULL);
	
	set_audio_callback(audio_api_callback);
	
	init_audio();
	
}

void svc_close(){
	close_audio();
	encoder_destroy(svc_encoder);
	packet_queue_destroy(packet_queue);
	incoming_close();
	free(svc_options);
}
