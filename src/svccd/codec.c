#include "codec.h"

decoder_t* decoder_create(int Fs, int frame_size, int byte_per_packet){
	int error;
	decoder_t* res_decoder = malloc(sizeof(decoder_t));
	
	res_decoder->celt_mode = celt_mode_create (Fs, frame_size, &error);
	res_decoder->byte_per_packet = byte_per_packet;
	res_decoder->celt_decoder = celt_decoder_create (mode, 1, &error);
	
	return res_decoder;
}

void decoder_destroy(decoder_t* decoder){
	 celt_decoder_destroy(decoder->celt_decoder);
	 celt_mode_destroy(decoder->celt_mode);
	 free(decoder);
}

encoder_t* encoder_create(int Fs, int frame_size, int byte_per_packet){
	int error;
	encoder_t* res_encoder = malloc(sizeof(encoder_t));
	
	res_encoder->celt_mode = celt_mode_create (Fs, frame_size, &error);
	res_encoder->byte_per_packet = byte_per_packet;
	res_encoder->celt_encoder = celt_encoder_create (mode, 1, &error);
	
	return res_decoder;
}

void encoder_destroy(encoder_t* encoder){
	 celt_encoder_destroy(encoder->celt_encoder);
	 celt_mode_destroy(encoder->celt_mode);
	 free(encoder);
}

int encoder_encode(encoder_t* encoder, const float* pcm, unsigned char* compressed){
	return celt_encode_float(encoder->celt_encoder, pcm, NULL, compressed, encoder->byte_per_packet);
}

int decoder_decode(decoder_t* decoder, unsigned char* data, int len, const float* pcm){
	return cel_decode_float(decoder->celt->decoder, data, len, pcm);
}
