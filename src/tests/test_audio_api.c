
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "audio_api.h"

#include <assert.h>

#define FRAME_SIZE 1024

svc_audio_data_t *i_data;


void reader_writer(svc_audio_data_t* input_audio_data, svc_audio_data_t* output_audio_data){
	memcpy(i_data->data, input_audio_data->data, sizeof(svc_sample_t) * FRAME_SIZE);
	memcpy(output_audio_data->data, i_data->data, sizeof(svc_sample_t) * FRAME_SIZE);
}


int main() {
	
	i_data = svc_audio_data_create(FRAME_SIZE);

	svc_set_audio_callback(reader_writer);

	svc_init_audio(48000, FRAME_SIZE);

	puts("Your mic input should be loopback to your speakers.\nEnter some text when done with test.");

	getchar();
	svc_close_audio();
	return 0;
}

