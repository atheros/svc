#ifndef __CODEC_H_
#define __CODEC_H_ 

#include <celt/celt.h>

typedef struct _decoder_t {
	CELTDecoder* celt_decoder;
	CELTMode* celt_mode;
	int byte_per_packet;
} decoder_t;

typedef struct _encoder_t {
	CELTEncoder* celt_encoder;
	CELTMode* celt_mode;
	int byte_per_packet;
} encoder_t;

/* Creates a new decoder. Must have a unique decoder for every channel. */
decoder_t* svc_decoder_create(int Fs, int frame_size, int byte_per_packet);

/* Destroys the decoder. */
void svc_decoder_destroy(decoder_t* decoder);

/* Creates a new encoder. Must have a unique encoder for every channel. */
encoder_t* svc_encoder_create(int Fs, int frame_size, int byte_per_packet);

/* Destroys the encoder. */
void svc_encoder_destroy(encoder_t* encoder);

int svc_encoder_encode(encoder_t* encoder, const float* pcm, unsigned char* compressed);

int svc_decoder_decode(decoder_t* decoder, unsigned char* data, int len, float* pcm);

#endif
