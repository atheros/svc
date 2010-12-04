/*
 * protocol.h
 *
 *  Created on: 04-12-2010
 *      Author: przemek
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "server.h"

/**
 * Send a raw error to a peer.
 *
 * This kind of error means the client should drop the connection.
 */
void protocol_raw_error(Server* server, ENetPeer* peer, const char* error);

#endif /* PROTOCOL_H_ */
