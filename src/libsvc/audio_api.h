
#ifndef __AUDIO_API_H_
#define __AUDIO_API_H_ 

#include "audio.h"

/*  Callback which is called by audio_api whenever it captures a portion of sound. */
typedef void(* capture_audio_callback_t)(audio_data_t* audio_data);

/*  Callback which is called by audio_api whenever it has to play a portion of sound. */
typedef void(* playback_audio_callback_t)(audio_data_t* audio_data);

/* Initializes the audio system and sets the callback thread running */
int init_audio (unsigned int rate, unsigned int frame_size);

/* Releases the audio system and stops the callback thread */
int close_audio();

/* Set the audio callbacks */
int set_audio_callbacks(capture_audio_callback_t capture_callback, playback_audio_callback_t playback_callback);

#endif
