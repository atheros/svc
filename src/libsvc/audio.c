#include "audio.h"


svc_audio_data_t* svc_audio_data_create(unsigned int size){
	svc_audio_data_t* res_data = malloc(sizeof(svc_audio_data_t));
	res_data->size = size;
	res_data->data = malloc(size*sizeof(svc_sample_t));
	return res_data;
}

void svc_audio_data_destroy(svc_audio_data_t* audio_data){
	free(audio_data->data);
	free(audio_data);
}

svc_audio_data_t* svc_audio_fake_data_create(unsigned int size){
	svc_audio_data_t* res_data = malloc(sizeof(svc_audio_data_t));
	res_data->size = size;
	return res_data;
}

void svc_audio_fake_data_destroy(svc_audio_data_t* audio_data){
	free(audio_data);
}

int svc_audio_data_copy(svc_audio_data_t* dest_data, svc_audio_data_t* source_data){
	if(dest_data->size!=source_data->size) return -1;
	memcpy(dest_data->data, source_data->data, source_data->size*sizeof(svc_sample_t));
	return 0;
}

void svc_clear_audio_data(svc_audio_data_t* audio_data){
	unsigned int i;
	for (i=0; i < audio_data->size; i++) audio_data->data[i] = 0;
}

int svc_mix_audio_streams(unsigned int count, svc_audio_data_t** input_streams, svc_audio_data_t* result_stream) {
	unsigned int i;
	unsigned int k;
	
	for (k = 0; k < result_stream->size; k++) /* filling the output stream with zeros */
		result_stream->data[k] = 0;

	for (i = 0; i < count; i++){
		
		if (input_streams[i]->size != result_stream->size) 
			return -1;
		
		for (k = 0; k < result_stream->size; k++){ 
			result_stream->data[k] += input_streams[i]->data[k];
		}
			
	}
	
	return 0;
}
