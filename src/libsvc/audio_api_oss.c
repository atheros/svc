#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <error.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <soundcard.h>

#include "audio_api.h"
#include "thread.h"

audio_callback_t pa_interface_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

static int fd;
static uint_fast32_t fs;	// frame size
static thread_t rt;

static void oss_open(uint_fast16_t rate) {
	assert(fd == 0);	// making sure we don't do double-open
	const char* dsp = "/dev/dsp";	// TODO device setting, also see http://manuals.opensound.com/developer/SNDCTL_AUDIOINFO.html
	// TODO duplexity check, see http://manuals.opensound.com/developer/full_duplex.html
	if ((fd = open(dsp, O_RDWR)) == -1) {
		error(1, errno, "failed to open %s", dsp);
	}

	int i, j;
#define SET(what, to) i = to; \
	j = ioctl(fd, what, &i); \
	if (j == -1) \
		error(1, errno, "failed to set %s to %i", #what, i);

	SET(SNDCTL_DSP_CHANNELS, 1);
	SET(SNDCTL_DSP_SETFMT, AFMT_S16_NE);
	SET(SNDCTL_DSP_SPEED, rate);
}

static void *reader(void *_) {
	size_t s = fs * 2;
	int16_t *buf = malloc(s);
	assert(buf);
	while (true) {
		ssize_t i;

		i = 0;
		while (i < s) {
			i += read(fd, (char *)buf + i, s - i);
			if (i < s) perror("failed to get sound");
		}

		for (i = 0; i < fs; i++)
			input_audio_data->data[i] = buf[i];

		pa_interface_callback(input_audio_data, output_audio_data);

		for (i = 0; i < fs; i++)
			buf[i] = output_audio_data->data[i];

		i = 0;
		while (i < s) {
			i += write(fd, (char *)buf + i, s - i);
			if (i < s) perror("failed to output sound");
		}
	}
	return NULL;
}

int init_audio(uint_fast16_t rate, uint_fast32_t frame_size) {
	input_audio_data  = malloc(sizeof(audio_data_t));
	output_audio_data = malloc(sizeof(audio_data_t));
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;
	input_audio_data->data = malloc(sizeof(sample_t) * frame_size);
	output_audio_data->data = malloc(sizeof(sample_t) * frame_size);

	oss_open(rate);

	fs = frame_size;
	thread_create(&rt, reader, NULL);	// undocumented crap
	assert(rt > 0);

	return 0;
}

int close_audio() {
	thread_exit(rt);
	assert(rt != 0);
	assert(rt = 0);
	return 0;
}

int set_audio_callback(audio_callback_t audio_callback) {
	pa_interface_callback = audio_callback;
	return 0;
}

