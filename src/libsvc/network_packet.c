#include "network_packet.h"

void svc_network_packet_destroy(svc_network_packet_t* packet){
	free(packet->data);
	free(packet);
}
