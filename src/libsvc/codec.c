#include "codec.h"
#include <string.h>
#include <stdlib.h>
int ololo_size;

decoder_t* svc_decoder_create(int Fs, int frame_size, int byte_per_packet){
	int error;
	decoder_t* res_decoder = malloc(sizeof(decoder_t));
	
	ololo_size = frame_size;
	res_decoder->celt_mode = celt_mode_create (Fs, frame_size, &error);
	res_decoder->byte_per_packet = byte_per_packet;
	res_decoder->celt_decoder = celt_decoder_create (res_decoder->celt_mode, 1, &error);
	
	return res_decoder;
}

void svc_decoder_destroy(decoder_t* decoder){
	 celt_decoder_destroy(decoder->celt_decoder);
	 celt_mode_destroy(decoder->celt_mode);
	 free(decoder);
}

encoder_t* svc_encoder_create(int Fs, int frame_size, int byte_per_packet){
	int error;
	encoder_t* res_encoder = malloc(sizeof(encoder_t));
	
	ololo_size = frame_size;
	res_encoder->celt_mode = celt_mode_create (Fs, frame_size, &error);
	res_encoder->byte_per_packet = byte_per_packet;
	res_encoder->celt_encoder = celt_encoder_create (res_encoder->celt_mode, 1, &error);
	
	return res_encoder;
}

void svc_encoder_destroy(encoder_t* encoder){
	 celt_encoder_destroy(encoder->celt_encoder);
	 celt_mode_destroy(encoder->celt_mode);
	 free(encoder);
}

int svc_encoder_encode(encoder_t* encoder, const float* pcm, unsigned char* compressed){
	return celt_encode_float(encoder->celt_encoder, pcm, NULL, compressed, encoder->byte_per_packet);
}

int svc_decoder_decode(decoder_t* decoder, unsigned char* data, int len, float* pcm){
	return celt_decode_float(decoder->celt_decoder, data, len, pcm);
}
