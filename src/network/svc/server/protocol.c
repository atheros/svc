/*
 * protocol.c
 *
 *  Created on: 04-12-2010
 *      Author: przemek
 */

#include "protocol.h"

void protocol_raw_error(Server* server, ENetPeer* peer, const char* error) {
	ENetPacket* p;
	dstring* s;

	s = dfromcs(error);
	p = enet_packet_create(s->data, s->len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, p);
}
