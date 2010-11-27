#ifndef __AUDIO_H_
#define __AUDIO_H_ 

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef float svc_sample_t;

typedef struct _svc_audio_data_t {
	unsigned int size;
	svc_sample_t *data;
} svc_audio_data_t;

svc_audio_data_t* svc_audio_data_create(unsigned int size);

/* Same thing as create but does not allocate memory for the actual data. */
svc_audio_data_t* svc_audio_fake_data_create(unsigned int size);

/* Cleans up audio data making it silent */
void svc_clear_audio_data(svc_audio_data_t* audio_data);

void svc_audio_data_destroy(svc_audio_data_t* audio_data);

/* Same thing as destroy but does not free the memory for actual data. */
void svc_audio_fake_data_destroy(svc_audio_data_t* audio_data);

/* Make sure the destination is initiated before calling this.
 * Returns 0 on success. */
int svc_audio_data_copy(svc_audio_data_t* dest_data, svc_audio_data_t* source_data);

/* mixes audio streams.
 * count         - number of streams to mix
 * input_streams - an array of pointer to audio streams
 * result_stream - a pointer to place the result
 * return value  - 0 on success */
int svc_mix_audio_streams(unsigned int count, svc_audio_data_t** input_streams, svc_audio_data_t* result_stream);

#endif
