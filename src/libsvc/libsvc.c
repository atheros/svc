#include <stdlib.h>
#include <string.h>

#include "libsvc.h"
#include "packet_cage.h"
#include "packet_queue.h"
#include "thread.h"
#include "audio.h"
#include "audio_api.h"

static svc_options_t* svc_options;

static packet_time_t svc_time;

static svc_send_callback_t svc_send_callback;

static packet_queue_t* packet_queue;

static encoder_t* svc_encoder;

static thread_t svc_send_thread;

static int svc_running;

static void audio_api_callback(svc_audio_data_t* input_audio_data, svc_audio_data_t* output_audio_data){
	packet_queue_push_data(packet_queue, input_audio_data);
	svc_request_incoming_audio(output_audio_data);
}

static network_packet_t* create_network_packet_from_audio(svc_audio_data_t* audio_data){
	network_packet_t* network_packet = malloc(sizeof(network_packet_t));
	network_packet->data = malloc(sizeof(unsigned char)*svc_options->byte_per_packet);
	network_packet->data_len = svc_options->byte_per_packet;
	svc_encoder_encode(svc_encoder, audio_data->data, network_packet->data);
	svc_time = time_inc(svc_time);
	network_packet->time = svc_time;
	return network_packet;
}

static void *send_network_thread_function( void *ptr ){
	svc_audio_data_t* audio_data = svc_audio_data_create(svc_options->frame_size);
	while(svc_running){
		packet_queue_pop_data(packet_queue, audio_data);
		network_packet_t* network_packet = create_network_packet_from_audio(audio_data);
		svc_send_callback(network_packet);
		network_packet_destroy(network_packet);
	}
	return NULL;
}


void svc_init(svc_send_callback_t send_callback){
	svc_time = 0;
	svc_running = 1;
	svc_options = malloc(sizeof(svc_options_t));
	svc_options->frame_size = 1024;
	svc_options->sample_rate = 44100;
	svc_options->byte_per_packet = 256;
	
	svc_encoder = svc_encoder_create(svc_options->sample_rate, svc_options->frame_size, svc_options->byte_per_packet);
	packet_queue = packet_queue_create(100, svc_options->frame_size);
	
	svc_incoming_init(svc_options);
	
	svc_send_callback = send_callback;		
	
	thread_create(&svc_send_thread, send_network_thread_function, NULL);
	
	svc_set_audio_callback(audio_api_callback);
	
	svc_init_audio(svc_options->sample_rate, svc_options->frame_size);
	
}

void svc_close(){
	svc_running = 0;
	thread_detach(svc_send_thread);
	svc_close_audio();
	svc_encoder_destroy(svc_encoder);
	packet_queue_destroy(packet_queue);
	svc_incoming_close();
	free(svc_options);
}
