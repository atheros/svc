
#ifndef __AUDIO_API_H_
#define __AUDIO_API_H_ 

#include "audio.h"

/* Memory is reused. Don't bother freeing anything returned from or passed to callbacks. */

/* Callback which is called by audio_api to send sound to network */
typedef void(* audio_input_callback_t)(audio_data_t* audio_data);

/* Callback which is called by audio_api to get sound packet to output to speakers */
typedef audio_data_t* (* audio_output_callback_t)();

/* Initializes the audio system and sets the callback thread running */
int init_audio();

/* Releases the audio system and stops the callback thread */
int close_audio();

/* Set the callback to send packets to network */
int set_input_callback(audio_input_callback_t input_callback);

/* Set the callback to send packets to speakers */
int set_output_callback(audio_output_callback_t output_callback);

#endif

