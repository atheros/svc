
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "audio_api.h"

#include <assert.h>

#define FRAME_SIZE 1024

audio_data_t *i_data;

void shit_read(audio_data_t *packet) {
	assert(i_data != NULL);
	assert(i_data->data != NULL);
	assert(packet != NULL);
	assert(packet->data != NULL);
	memcpy(i_data->data, packet->data, sizeof(sample_t) * FRAME_SIZE);
}

void shit_write(audio_data_t *packet) {
	assert(i_data != NULL);
	assert(i_data->data != NULL);
	assert(packet != NULL);
	assert(packet->data != NULL);
	memcpy(packet->data, i_data->data, sizeof(sample_t) * FRAME_SIZE);
}

int main() {
	
	i_data = audio_data_create(FRAME_SIZE);

	set_audio_callbacks(shit_read, shit_write);

	init_audio(48000, FRAME_SIZE);

	puts("Your mic input should be loopback to your speakers.\nEnter some text when done with test.");

	getchar();
	close_audio();
	return 0;
}

