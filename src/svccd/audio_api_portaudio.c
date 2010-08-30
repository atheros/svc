
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

	printf("Callback called");
	/* need to somehow assert that packet->size == framesPerBuffer */
	audio_data_t* packet = pa_output_callback();
	memcpy(outputBuffer, packet->data, sizeof(float) * framesPerBuffer);

	free(packet->data);

	packet->data = (float*)inputBuffer;

	pa_input_callback(packet);
	
}

int init_audio () {
	printf("Initializing portaudio...");
	err = Pa_Initialize();
	DO_PA_ERROR;

	err = Pa_OpenDefaultStream( &stream,
			1,          /* input */
			1,          /* output */
			paFloat32,  /* 32 bit floating point output */
			SAMPLE_RATE,
			256,        /* frames per buffer */
			pa_callback, /* callback function */
			NULL );
	DO_PA_ERROR;

	err = Pa_StartStream( stream );
	DO_PA_ERROR;
	printf(" done.\n");
	return 0;
}

int close_audio () {
	printf("Closing portaudio...");
	err = Pa_StopStream(stream);
	DO_PA_ERROR;
	err = Pa_CloseStream( stream );
	DO_PA_ERROR;
	err = Pa_Terminate();
	DO_PA_ERROR;
	printf(" done.\n");
	return 0;
}

int set_input_callback(audio_input_callback_t input_callback) {
	pa_input_callback = input_callback;
}

int set_output_callback(audio_output_callback_t output_callback) {
	pa_output_callback = output_callback;
}

