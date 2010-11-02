
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "audio_api.h"


PaError err;
PaStream *stream;

audio_callback_t pa_interface_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

#define DO_PA_ERROR if (err != paNoError) { \
		printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) ); \
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


int init_audio (uint_fast16_t rate, uint_fast32_t frame_size) {
	
	printf("Initializing portaudio...");
	err = Pa_Initialize();
	DO_PA_ERROR;
	
	input_audio_data  = malloc(sizeof(audio_data_t));
	output_audio_data = malloc(sizeof(audio_data_t));
	
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

int set_audio_callback(audio_callback_t audio_callback) {
	pa_interface_callback = audio_callback;
	return 0;
}
