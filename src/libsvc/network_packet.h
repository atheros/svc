#ifndef __NETWORK_PACKET_H_
#define __NETWORK_PACKET_H_ 
#include <stddef.h>
#include <packet_cage.h>

typedef struct _network_packet_t {
	unsigned char* data;
	size_t data_len;
	packet_time_t time;
} network_packet_t;

void network_packet_destroy(network_packet_t* packet);

/* size_t network_packet_size(network_packet_t* network_packet); */

/* int network_packet_serialize(network_packet_t* network_packet, unsigned char* data); */

/* network_packet_t* network_packet_deserialize(unsigned char* data, size_t size); */

#endif /* __NETWORK_PACKET_H_ */
