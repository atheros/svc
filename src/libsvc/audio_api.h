
#ifndef __AUDIO_API_H_
#define __AUDIO_API_H_ 

#include "audio.h"

/*  Callback which is called by audio_api on every sound capture/output */
typedef void(* svc_audio_callback_t)(svc_audio_data_t* input_audio_data, svc_audio_data_t* output_audio_data);

/* Initializes the audio system and sets the callback thread running */
int svc_init_audio (unsigned int rate, unsigned int frame_size);

/* Releases the audio system and stops the callback thread */
int svc_close_audio();

/* Set the audio callback */
int svc_set_audio_callback(svc_audio_callback_t callback);

#endif
