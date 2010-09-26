
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_api.h"
#include "packet_cage.h"
#include "packet_queue.h"
#include "thread.h"

packet_cage_t* cage;
packet_queue_t* queue;

packet_time_t t;


void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	packet_queue_push_data(queue, input_packet);
	audio_data_t *data = packet_cage_get_data(cage);
	if(data!=NULL){
		memcpy(output_packet->data, data->data, sizeof(float) * 256); 
		audio_data_destroy(data);
	 }
	
}

void* worker(void* param) {
	while(1){
		audio_data_t *data = audio_data_create(256);
		packet_queue_pop_data(queue, data);
		t = time_inc(t);
		packet_cage_put_data(cage, data, t); 
	}
}

int main() {
	t=0;
	cage = packet_cage_create();
	queue = packet_queue_create(100, 256);
	thread_t thread;
	thread_create(&thread, worker, NULL);
	set_audio_callback(callback);
	
	
	init_audio();

	printf("Your mic input should loopback to your speakers.\nEnter some text when done with test.\n");

	int xxx;
	scanf("%d", &xxx);
	close_audio();
	packet_cage_destroy(cage);
	packet_queue_destroy(queue);
	return 0;
}
