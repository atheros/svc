#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "audio_api.h"


static PaError err;
static PaStream *stream;

static svc_audio_callback_t pa_interface_callback;

static svc_audio_data_t* input_audio_data;
static svc_audio_data_t* output_audio_data;

#define DO_PA_ERROR if (err != paNoError) { \
		fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText( err ) ); \
		return -1; \
	}


static int pa_callback( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData ) {

	input_audio_data->data = (float*)inputBuffer;
	output_audio_data->data = (float*)outputBuffer;
	
	pa_interface_callback(input_audio_data, output_audio_data);
	return 0;
	
}


int svc_init_audio(unsigned int rate, unsigned int frame_size) {
	
	printf("Initializing portaudio...\n");
	err = Pa_Initialize();
	DO_PA_ERROR;
	
	input_audio_data  = malloc(sizeof(svc_audio_data_t));
	output_audio_data = malloc(sizeof(svc_audio_data_t));
	
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;
	
	err = Pa_OpenDefaultStream( &stream,
			1,          /* input */
			1,          /* output */
			paFloat32,  /* 32 bit floating point output */
			rate,
			frame_size,        /* frames per buffer */
			pa_callback, /* callback function */
			NULL );
	DO_PA_ERROR;

	err = Pa_StartStream( stream );
	DO_PA_ERROR;
	printf("done.\n");
	return 0;
}

int svc_close_audio () {
	printf("Closing portaudio...\n");
	err = Pa_StopStream(stream);
	DO_PA_ERROR;
	err = Pa_CloseStream( stream );
	DO_PA_ERROR;
	err = Pa_Terminate();
	DO_PA_ERROR;
	printf("done.\n");
	return 0;
}

int svc_set_audio_callback(svc_audio_callback_t audio_callback) {
	pa_interface_callback = audio_callback;
	return 0;
}
