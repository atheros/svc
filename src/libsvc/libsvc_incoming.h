#ifndef __LIBSVC_INCOMING_H_
#define __LIBSVC_INCOMING_H_ 

#include "network_packet.h"
#include "libsvc_options.h"
#include "codec.h"
#include "packet_cage.h"

typedef struct _peer_t {
	decoder_t* decoder;
	packet_cage_t* cage;
	struct _peer_t* next;
	struct _peer_t* prev;
} peer_t;	

void svc_request_incoming_audio(audio_data_t* output_audio_data);

/* Call this function when you want to listen to one more peer*/
peer_t* svc_peer_join();

/* Call this function when you don't have to listen to the peer anymore */
void svc_peer_leave(peer_t* peer);

void svc_packet_recieve(network_packet_t* packet, peer_t* peer);

void svc_incoming_init(svc_options_t* options);

void svc_incoming_close();

#endif /* __LIBSVC_INCOMING_H_ */
