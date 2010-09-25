
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_api.h"
#include "packet_cage.h"

packet_cage_t* cage;

packet_time_t t;

int shouldBlock;

void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	if(!shouldBlock) { 
		t = time_inc(t);
		packet_cage_put_data(cage, input_data, t); 
		t = time_inc(t);
		packet_cage_put_data(cage, input_data, t); 
		t = time_inc(t);
		packet_cage_put_data(cage, input_data, t); 
		shouldBlock=1;
	} else shouldBlock=0;
	audio_data_t *data = packet_cage_get_data(cage);
	if(data!=NULL) memcpy(output_packet->data, data->data, sizeof(float) * 256); 
}

void main() {
	t=0;
	shouldBlock = 0;
	cage = packet_cage_create();
	set_audio_callback(callback);

	init_audio();

	printf("Your mic input should be loopback to your speakers with some distortion.\nEnter some text when done with test.\n");

	int xxx;
	scanf("%d", &xxx);
	close_audio();
	packet_cage_destroy(cage);
}
