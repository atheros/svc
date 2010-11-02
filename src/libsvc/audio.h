#ifndef __AUDIO_H_
#define __AUDIO_H_ 

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef float sample_t;

typedef struct _audio_data_t {
	unsigned int size;
	sample_t *data;
} audio_data_t;

audio_data_t* audio_data_create(unsigned int size);

/* Cleans up audio data making it silent */
void clear_audio_data(audio_data_t* audio_data);

void audio_data_destroy(audio_data_t* audio_data);

/* Make sure the destination is initiated before calling this.
 * Returns 0 on success. */
int audio_data_copy(audio_data_t* dest_data, audio_data_t* source_data);

/* mixes audio streams.
 * count         - number of streams to mix
 * input_streams - an array of pointer to audio streams
 * result_stream - a pointer to place the result
 * return value  - 0 on success */
int mix_audio_streams(unsigned int count, audio_data_t** input_streams, audio_data_t* result_stream);

#endif
