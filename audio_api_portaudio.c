
#include <string.h>
#include <stdio.h>
#include <portaudio.h>

#include "audio_api.h"
#include "list.h"

PaError err;
PaStream *stream;

#define SAMPLE_RATE (44100)

#define DO_PA_ERROR() { \
	if (err != paNoError) { \
		printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) ); \
		return -1; \
	} \
}

static int pa_callback( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData )
{
	/* Cast data passed through stream to our structure. */
	/* paTestData *data = (paTestData*)userData; */
	float *out = (float*)outputBuffer;
	memset(out, 0, framesPerBuffer * 2);
}

int init_audio () {
	err = Pa_Initialize();
	DO_PA_ERROR();

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
	DO_PA_ERROR();

	err = Pa_StartStream( stream );
	DO_PA_ERROR();
	return 0;
}

int close_audio () {
	err = Pa_StopStream(stream);
	DO_PA_ERROR();
	err = Pa_CloseStream( stream );
	DO_PA_ERROR();
	err = Pa_Terminate();
	DO_PA_ERROR();
	return 0;
}

