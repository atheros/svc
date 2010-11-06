#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <error.h>
#include <errno.h>

#include "audio_api.h"
#include "thread.h"

audio_callback_t process_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

int running;
thread_t rt;

static void *loop(void *arg) {
	while (running) {
		usleep(44100 * 1000 / 1024);
		process_callback(input_audio_data, output_audio_data);
	}
	return NULL;
}

int init_audio(uint_fast16_t rate, uint_fast32_t frame_size) {
	input_audio_data = malloc(sizeof(audio_data_t));
	input_audio_data->data = malloc(sizeof(float) * frame_size);
	memset(input_audio_data->data, 0, frame_size * sizeof(float));
	output_audio_data = malloc(sizeof(audio_data_t));
	output_audio_data->data = malloc(sizeof(float) * frame_size);
	memset(output_audio_data->data, 0, frame_size * sizeof(float));
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;

	running = 1;
	if (thread_create(&rt, loop, NULL) > 0) {
		fprintf(stderr, "Failed creating thread");
		return -1;
	}

	return 0;
}

int close_audio() {
	running = 0;
	thread_join(rt);
	return 0;
}

int set_audio_callback(audio_callback_t audio_callback) {
	process_callback = audio_callback;
	return 0;
}

