
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "audio_api.h"

#define FRAME_SIZE 1024

void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	memcpy(output_packet->data, input_packet->data, sizeof(sample_t) * FRAME_SIZE); 
	/*
	for (unsigned i = 0; i < FRAME_SIZE; i++) {
		output_packet->data[i] = sinf(i / (sample_t)FRAME_SIZE * 50) - 1.0;
	}
	*/
}

int main() {
	set_audio_callback(callback);

	init_audio(48000, FRAME_SIZE);

	puts("Your mic input should be loopback to your speakers.\nEnter some text when done with test.");

	getchar();
	close_audio();
	return 0;
}
