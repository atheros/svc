#include "audio.h"

int mix_audio_streams(unsigned int count, audio_data_t** input_streams, audio_data_t* result_stream) {
	unsigned int i;
	unsigned int k;
	
	for (i = 0; i < result_stream->size; i++) /* filling the output stream with zeros */
		result_stream->data[i] = 0;

	for (i = 0; i < count; i++){
		
		if (input_streams[i]->size != result_stream->size) 
			return -1;
		
		for (k = 0; k < result_stream->size; i++) 
			result_stream->data[k] += input_streams[i]->data[k];
			
	}
	
	return 0;
}
