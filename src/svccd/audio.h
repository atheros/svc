#ifndef __AUDIO_H_
#define __AUDIO_H_ 

typedef struct _audio_data_t {
	unsigned int size;
	float *data;
} audio_data_t; 

/* mixes audio streams.
 * count         - number of streams to mix
 * input_streams - an array of pointer to audio streams
 * result_stream - a pointer to place the result
 * return value  - 0 on success */
int mix_audio_streams(unsigned int count, audio_data_t** input_streams, audio_data_t* result_stream);

#endif

