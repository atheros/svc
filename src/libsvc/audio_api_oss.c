#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <error.h>
#include <errno.h>

#include <soundcard.h>

#include "audio_api.h"
#include "thread.h"

audio_callback_t pa_interface_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

static int fd;
static uint_fast32_t fs;
static thread_t rt;

static void oss_open(uint_fast16t rate) {
	const char* dsp = "/dev/dsp";
	if ((fd = open(dsp, O_RDWR)) == -1) {
		error(1, errno, "failed to open %s", dsp);
	}

	int i, j;
#define SET(what, to) i = to; \
	j = ioctl(fd, what, &i); \
	if (j == -1) \
		error(1, errno, "failed to set %s to %i", #what, i);

	SET(SNDCTL_DSP_CHANNELS, 1);
	SET(SNDCTL_DSP_SETFMT, AFMT_U8);
	SET(SNDCTL_DSP_SPEED, rate);
}

static void reader(void *) {
	size_t s = fs * 1;
	while (true) {
		assert(s == read(fd, &(input_audio_data->data), s));	// FIXME
		pa_interface_callback(input_audio_data, output_audio_data);
		assert(s == write(fd, &(output_audio_data->data), s));	// FIXME
	}
}

int init_audio(uint_fast16_t rate, uint_fast32_t frame_size) {
	input_audio_data  = malloc(sizeof(audio_data_t));
	output_audio_data = malloc(sizeof(audio_data_t));
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;

	oss_open(rate);

	fs = frame_size;
	assert((rt = thread_create(NULL, reader, NULL)) > 0);	// undocumented crap

	return 0;
}

int close_audio() {
	thread_exit(rt);
	return 0;
}

int set_audio_callback(audio_callback_t audio_callback) {
	pa_interface_callback = audio_callback;
	return 0;
}

