/*
 * server.h
 *
 *  Created on: 27-11-2010
 *      Author: przemek
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "peer.h"
#include "channels.h"


typedef struct {
	ENetHost* host;
	ENetPeer* server_peer;

	Channels* channels;
	PeerList* peers;
} Server;



#endif /* SERVER_H_ */
