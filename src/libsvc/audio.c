#include "audio.h"

audio_data_t* audio_data_create(unsigned int size){
	audio_data_t* res_data = malloc(sizeof(audio_data_t));
	res_data->size = size;
	res_data->data = malloc(size*sizeof(sample_t));
	return res_data;
}

void audio_data_destroy(audio_data_t* audio_data){
	free(audio_data->data);
	free(audio_data);
}

int audio_data_copy(audio_data_t* dest_data, audio_data_t* source_data){
	if(dest_data->size!=source_data->size) return -1;
	memcpy(dest_data->data, source_data->data, source_data->size*sizeof(sample_t));
	return 0;
}

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
