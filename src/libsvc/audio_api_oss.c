#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <stropts.h>

#include <error.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__OpenBSD__) || defined(__NetBSD__) // Stolen from mpd; seems sensible
#include <soundcard.h>
#else
/* using sys/soundcard.h to avoid adding /usr/include/sys/ to the include path
 * because it causes the nasty include loop[1]; it is against 4front's 
 * recommendations though[2]
 *
 * [1] http://sprunge.us/VZXW
 * [2] http://manuals.opensound.com/developer/programming.html */
#include <sys/soundcard.h>
#endif

#include "audio_api.h"
#include "thread.h"

static svc_audio_callback_t oss_callback;

static svc_audio_data_t* input_audio_data;
static svc_audio_data_t* output_audio_data;

static int fd;
static unsigned int fs;	/* frame size  */
static thread_t rt;
static int running;

static void oss_open(unsigned int rate) {
	assert(fd == 0);	/* making sure we don't do double-open */
	const char* dsp = "/dev/dsp";	/* TODO device setting, also see http://manuals.opensound.com/developer/SNDCTL_AUDIOINFO.html */
	/* TODO duplexity check, see http://manuals.opensound.com/developer/full_duplex.html */
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
	(void)_;
	size_t s = fs * 2;
	int16_t *buf = malloc(s);
	assert(buf);
	while (running) {
		size_t i;

		i = 0;
		while (i < s) {
			i += read(fd, (char *)buf + i, s - i);
			if (i < s) perror("failed to get sound");
		}

		for (i = 0; i < fs; i++)
			input_audio_data->data[i] = buf[i];

		oss_callback(input_audio_data, output_audio_data);

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

int svc_init_audio(unsigned int rate, unsigned int frame_size) {
	input_audio_data = svc_audio_data_create(frame_size);
	output_audio_data = svc_audio_data_create(frame_size);

	oss_open(rate);

	fs = frame_size;
	running = 1;
	thread_create(&rt, reader, NULL);
	thread_detach(rt);
	assert(rt > 0);

	return 0;
}

int svc_close_audio() {
	running = 0;
	thread_join(rt);
	assert(rt != 0);
	assert(rt = 0);

	svc_audio_data_destroy(input_audio_data);
	svc_audio_data_destroy(output_audio_data);
	return 0;
}

int svc_set_audio_callback(svc_audio_callback_t callback){
	oss_callback = callback;
	return 0;
}

