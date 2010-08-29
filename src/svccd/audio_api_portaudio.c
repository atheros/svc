
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "audio_api.h"

PaError err;
PaStream *stream;

audio_input_callback_t pa_input_callback;
audio_output_callback_t pa_output_callback;

#define SAMPLE_RATE (44100)

#define DO_PA_ERROR if (err != paNoError) { \
		printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) ); \
		return -1; \
	}

static int pa_callback( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData ) {

	/* need to somehow assert that packet->size == framesPerBuffer */
	audio_data_t* packet = pa_output_callback();
	memcpy(outputBuffer, packet->data, sizeof(float) * framesPerBuffer);

	packet->data = (float*)inputBuffer;

	pa_input_callback(packet);
	
}

int init_audio () {
	err = Pa_Initialize();
	DO_PA_ERROR;

	err = Pa_OpenDefaultStream( &stream,
			1,          /* input */
			2,          /* stereo output */
			paFloat32,  /* 32 bit floating point output */
			SAMPLE_RATE,
			256,        /* frames per buffer, i.e. the number
						   of sample frames that PortAudio will
						   request from the callback. Many apps
						   may want to use
						   paFramesPerBufferUnspecified, which
						   tells PortAudio to pick the best,
						   possibly changing, buffer size.*/
			pa_callback, /* this is your callback function */
			NULL );
	DO_PA_ERROR;

	err = Pa_StartStream( stream );
	DO_PA_ERROR;
	return 0;
}

int close_audio () {
	err = Pa_StopStream(stream);
	DO_PA_ERROR;
	err = Pa_CloseStream( stream );
	DO_PA_ERROR;
	err = Pa_Terminate();
	DO_PA_ERROR;
	return 0;
}

int set_input_callback(audio_input_callback_t input_callback) {
	pa_input_callback = input_callback;
}

int set_output_callback(audio_output_callback_t output_callback) {
	pa_output_callback = output_callback;
}

