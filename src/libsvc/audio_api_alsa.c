#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <error.h>
#include <errno.h>

#include <alsa/asoundlib.h>

#include "audio_api.h"
#include "thread.h"

static capture_audio_callback_t alsa_capture_callback;
static playback_audio_callback_t alsa_playback_callback;

static audio_data_t* input_audio_data;
static audio_data_t* output_audio_data;

static unsigned int frame_size;

static snd_pcm_t *pcm_playback_handle;
static snd_pcm_t *pcm_capture_handle;
static snd_pcm_hw_params_t *playback_hwparams;
static snd_pcm_hw_params_t *capture_hwparams;

static thread_t rt, wt;
static int running;

static int alsa_open(unsigned int rate) {
	snd_pcm_hw_params_malloc(&playback_hwparams);
	snd_pcm_hw_params_malloc(&capture_hwparams);

	char *playback_device = getenv("SVC_ALSA_PLAYBACK");
	if (!playback_device) {
		playback_device = "default";
	}

	char *capture_device = getenv("SVC_ALSA_CAPTURE");
	if (!capture_device) {
		capture_device = "default";
	}

	if (snd_pcm_open(&pcm_playback_handle, playback_device, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		fprintf(stderr, "Error opening playback PCM device %s\n", playback_device);
		return(-1);
	}

	if (snd_pcm_hw_params_any(pcm_playback_handle, playback_hwparams) < 0) {
		fprintf(stderr, "Can not configure this PCM device.\n");
		return(-1);
	}

	if (snd_pcm_open(&pcm_capture_handle, capture_device, SND_PCM_STREAM_CAPTURE, 0) < 0) {
		fprintf(stderr, "Error opening capture PCM device %s\n", capture_device);
		return(-1);
	}

	if (snd_pcm_hw_params_any(pcm_capture_handle, capture_hwparams) < 0) {
		fprintf(stderr, "Can not configure this PCM device.\n");
		return(-1);
	}

	if (snd_pcm_hw_params_set_access(pcm_playback_handle, playback_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) {
		fprintf(stderr, "Error setting access\n");
		return(-1);
	};

	if (snd_pcm_hw_params_set_access(pcm_capture_handle, capture_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) {
		fprintf(stderr, "Error setting access\n");
		return(-1);
	};

	unsigned int exact_rate;
	if (snd_pcm_hw_params_set_format(pcm_playback_handle, playback_hwparams, SND_PCM_FORMAT_FLOAT_LE) < 0) {
		fprintf(stderr, "Error setting format.\n");
		return(-1);
	}

	if (snd_pcm_hw_params_set_format(pcm_capture_handle, capture_hwparams, SND_PCM_FORMAT_FLOAT_LE) < 0) {
		fprintf(stderr, "Error setting format.\n");
		return(-1);
	}

	exact_rate = rate;
	if (snd_pcm_hw_params_set_rate_near(pcm_playback_handle, playback_hwparams, &exact_rate, 0) < 0) {
		fprintf(stderr, "Error setting rate.\n");
		return(-1);
	}
	if (rate != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n", rate, exact_rate);
	}

	exact_rate = rate;
	if (snd_pcm_hw_params_set_rate_near(pcm_capture_handle, capture_hwparams, &exact_rate, 0) < 0) {
		fprintf(stderr, "Error setting rate.\n");
		return(-1);
	}
	if (rate != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n", rate, exact_rate);
	}

	if (snd_pcm_hw_params_set_channels(pcm_playback_handle, playback_hwparams, 1) < 0) {
		fprintf(stderr, "Error setting channels.\n");
		return(-1);
	}

	if (snd_pcm_hw_params_set_channels(pcm_capture_handle, capture_hwparams, 1) < 0) {
		fprintf(stderr, "Error setting channels.\n");
		return(-1);
	}

	if (snd_pcm_hw_params(pcm_playback_handle, playback_hwparams) < 0) {
		fprintf(stderr, "Error setting playback HW params.\n");
		return(-1);
	}

	if (snd_pcm_hw_params(pcm_capture_handle, capture_hwparams) < 0) {
		fprintf(stderr, "Error setting capture HW params.\n");
		return(-1);
	}

	snd_pcm_hw_params_free (playback_hwparams);
	snd_pcm_hw_params_free (capture_hwparams);
	return 0;
}


static void *reader(void *arg) {
	snd_pcm_prepare(pcm_capture_handle);
	int err;
	while (running) {
		/* read */
		while ((err = snd_pcm_readi(pcm_capture_handle, input_audio_data->data, frame_size * 4)) < 0) {
			snd_pcm_prepare(pcm_capture_handle);
			fprintf(stderr, "Capture error: %s\n", snd_strerror(err));
		}
		
		alsa_capture_callback(input_audio_data);
	}
	return NULL;
}

static void *writer(void *arg) {
	snd_pcm_prepare(pcm_playback_handle);
	int err;
	while (running) {
		/* write */
		alsa_playback_callback(output_audio_data);
		while ((err = snd_pcm_writei(pcm_playback_handle, output_audio_data->data, frame_size)) < 0) {
			snd_pcm_prepare(pcm_playback_handle);
			fprintf(stderr, "Playback error: %s\n", snd_strerror(err));
		}
	}
	return NULL;
}

int init_audio(unsigned int rate, unsigned int frame_size_i) {
	frame_size = frame_size_i;
	
	input_audio_data = audio_data_create(frame_size);
	output_audio_data = audio_data_create(frame_size);

	alsa_open(rate);

	running = 1;
	if (thread_create(&rt, reader, NULL) > 0) {
		fprintf(stderr, "Failed creating thread");
		return -1;
	}
	if (thread_create(&wt, writer, NULL) > 0) {
		fprintf(stderr, "Failed creating thread");
		return -1;
	}
	thread_detach(rt);
	thread_detach(wt);

	return 0;
}

int close_audio() {
	running = 0;
	thread_join(rt);
	thread_join(wt);
	snd_pcm_close(pcm_playback_handle);
	snd_pcm_close(pcm_capture_handle);
	audio_data_destroy(input_audio_data);
	audio_data_destroy(output_audio_data);
	return 0;
}

int set_audio_callbacks(capture_audio_callback_t capture_callback, 
                        playback_audio_callback_t playback_callback){
	alsa_capture_callback = capture_callback;
	alsa_playback_callback = playback_callback;
	return 0;
}

