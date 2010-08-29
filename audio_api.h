
#ifndef __AUDIO_API_H_
#define __AUDIO_API_H_ 
#include "audio.h"

typedef int(* audio_input_callback_t)(audio_data_t* audio_data);

typedef audio_data_t* (* audio_output_callback_t)();

int init_audio();

int close_audio();

int set_input_callback(audio_input_callback_t input_callback);

int set_output_callback(audio_output_callback_t output_callback);

#endif

