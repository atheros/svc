
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_api.h"
#include "packet_cage.h"
#include "audio.h"


/* TODO:
 * fis this test
 */

/*
packet_cage_t* cage;

packet_time_t t;

int shouldBlock;


void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	if(!shouldBlock) { 		
		t = time_inc(t);
		audio_data_t* tmp_data = audio_data_create(1024);
		audio_data_copy(tmp_data, input_packet);		
		packet_cage_put_data(cage, tmp_data, t); 
		audio_data_copy(tmp_data, input_packet);		
		packet_cage_put_data(cage, tmp_data, t); 
		audio_data_copy(tmp_data, input_packet);		
		packet_cage_put_data(cage, tmp_data, t); 
		shouldBlock=1;
	} else shouldBlock=0;
	audio_data_t *data = packet_cage_get_data(cage);
	if(data!=NULL){ 
		memcpy(output_packet->data, data->data, sizeof(float) * 1024); 
		audio_data_destroy(data);
	}
}


int main() {
	t=0;
	shouldBlock = 0;
	cage = packet_cage_create(5);
	
	set_audio_callback(callback);

	init_audio(44100, 1024);

	printf("Your mic input should loopback to your speakers with some distortion.\nEnter some text when done with test.\n");

	int xxx;
	scanf("%d", &xxx);
	close_audio();
	packet_cage_destroy(cage);
	return 0;
}
*/

int main(){
	printf("this test is depricated...");
}
