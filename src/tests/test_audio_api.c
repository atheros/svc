
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_api.h"


void callback(audio_data_t *input_packet, audio_data_t *output_packet) {
	memcpy(output_packet->data, input_packet->data, sizeof(float) * 1024); 
}

int main() {
	set_audio_callback(callback);

	init_audio();

	printf("Your mic input should be loopback to your speakers.\nEnter some text when done with test.\n");

	int xxx;
	scanf("%d", &xxx);
	close_audio();
	return 0;
}
