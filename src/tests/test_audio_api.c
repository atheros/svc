
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_api.h"

audio_data_t *current_data;

audio_data_t *output_callback() {
	printf("output called");
	return current_data;
}

void input_callback(audio_data_t *packet) {
	printf("input called");
	current_data = packet;
	float *data = current_data->data;
	current_data->data = malloc(sizeof(float) * 256);
	memcpy(current_data->data, data, sizeof(float) * 256);
}

void main() {

	current_data = malloc(sizeof(audio_data_t));
	current_data->size = 256;
	current_data->data = malloc(sizeof(float) * 256);

	set_input_callback(input_callback);
	set_output_callback(output_callback);

	init_audio();

	int xxx;
	scanf("%d", &xxx);
	close_audio();

}
