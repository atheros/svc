
#ifndef __AUDIO_API_H_
#define __AUDIO_API_H_ 

#include "audio.h"

/*  Callback which is called by audio_api  */
typedef void(* audio_callback_t)(audio_data_t* input_audio_data, audio_data_t* output_audio_data);

/* Initializes the audio system and sets the callback thread running */
int init_audio();

/* Releases the audio system and stops the callback thread */
int close_audio();

/* Set the audio callback */
int set_audio_callback(audio_callback_t input_callback);


#endif
