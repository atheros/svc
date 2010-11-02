#include "network_packet.h"

void network_packet_destroy(network_packet_t* packet){
	free(packet->data);
	free(packet);
}
