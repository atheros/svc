
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "audio_api.h"

/* TODO:
 * Please fix this test
 * /

/*
#define FRAME_SIZE 1024

void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	memcpy(output_packet->data, input_packet->data, sizeof(sample_t) * FRAME_SIZE); 

}

*/

int main() {
	
	/*
	set_audio_callback(callback);

	init_audio(48000, FRAME_SIZE);

	puts("Your mic input should be loopback to your speakers.\nEnter some text when done with test.");

	getchar();
	close_audio();
	*/
	printf("This test is depricated. Please fix it...");
	return 0;
}
