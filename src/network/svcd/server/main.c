/*
 * main.c
 *
 *  Created on: 2010-06-27
 *      Author: przemek
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "enetcheck.h"
#include "config.h"
#include "dstr.h"

/*
 * Get config file path.
 *
 * Returns a malloced string or NULL.
 *
 * You must free the string after you're done with it.
 */
/*
static char* get_config_file_path() {
	char* tmp;
	char* out;

	tmp = getenv("SVCD_CONFIG");
	if (tmp == NULL) {
		out = (char*)malloc(strlen(tmp)+1);
		strcpy(out, tmp);
		return out;
	}

	tmp = getenv('XDG_CONFIG_HOME');
	if (tmp == NULL) {
		out = (char*)malloc(strlen(tmp) + 32);
		strcpy(out, tmp);
		strcat(out, '/svcd.conf');
		return out;
	}

	return NULL;
}
*/

#if SVCSERVER_MAX_CLIENTS > 255
#  error Fix SVCSERVER_MAX_CLIENTS please. Maximum number of clients is 255
#endif

#define SERVER_HOST		"0.0.0.0"

typedef struct {
	int id; /* index, stored in peer->data */
	int used;
	int auth; /* is client authorized */
	char nick[32]; /* nickname */
	ENetPeer* peer;
} Peer;





static Peer peers[SVCSERVER_MAX_CLIENTS];
static ENetHost* server;






static void send_error(int peer_id, const char* message) {
	ENetPacket* p = enet_packet_create(message, strlen(message)+1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peers[peer_id].peer, 3, p);
}

static void send_auth(int peer_id) {
	ENetPacket* p;
	unsigned char buff[32];
	buff[0] = peer_id;
	buff[1] = SVCSERVER_MAX_CLIENTS;
	
	p = enet_packet_create(buff, 32, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peers[peer_id].peer, 0, p);
}

static void send_peer_list(int peer_id) {
	ENetPacket* p;
	int i, size = 0, count = 0;
	dstring* s;
	s = dnew();
	dcatc(s, SYSPACKAGE_LIST);
	dcatc(s, 0);
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (peers[i].used && peers[i].auth) {
			count++;
			dcatc(s, i);
			dcatc(s, strlen(peers[i].nick));
			dcatcs(s, peers[i].nick);
		}
	}
	
	s->data[1] = count;
	
	p = enet_packet_create(s->data, s->len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peers[peer_id].peer, 0, p);
	dfree(s);
}

static void send_new_peer_connected(int peer_id) {
	int i;
	dstring* s = dnew();
	dcatc(s, SYSPACKAGE_JOIN);
	dcatc(s, peer_id);
	dcatc(s, strlen(peers[peer_id].nick));
	dcatcs(s, peers[peer_id].nick);
	
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (i == peer_id || !peers[i].used || !peers[i].auth) {
			continue;
		}
		ENetPacket * packet = enet_packet_create(s->data, s->len, ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[i].peer, 1, packet);
	}
	dfree(s);
}

static void send_peer_disconnected(int peer_id) {
	int i;
	unsigned char buff[2];
	buff[0] = SYSPACKAGE_LEFT;
	buff[1] = peer_id;
	
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (i == peer_id || !peers[i].used || !peers[i].auth) {
			continue;
		}
		ENetPacket * packet = enet_packet_create(buff, 2, ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[i].peer, 1, packet);
	}
}


static void send_audio(int peer_id, void* data, unsigned int len) {
	ENetPacket* p;
	int i;
	
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (i == peer_id || !peers[i].used || !peers[i].auth) {
			continue;
		}
		
		p = enet_packet_create(NULL, len+1, 0);
		p->data[0] = peer_id;
		memcpy(p->data + 1, data, len);
		enet_peer_send(peers[i].peer, 2, p);
	}
}

static void handle_connection(ENetEvent* event) {
	int i;
	char buff[128];
	int peer_id = -1;
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (!peers[i].used) {
			peers[i].id = i;
			peers[i].peer = event->peer;
			peers[i].used = 1;
			peers[i].auth = 0;
			event->peer->data = &peers[i];
			peer_id = i;
			break;
		}
	}
	
	if (peer_id == -1) {
		/* FIXME: signal server full */
		enet_peer_disconnect(event->peer, 0);
		return;
	}
	
	/* log a nice message */
	enet_address_get_host_ip(&(event->peer->address), buff, 128);
	
	printf("Client #%i connected from %s:%u.\n", peer_id, buff, event->peer->address.port);
}



static void handle_disconnect(ENetEvent* event) {
	ENetPacket* packet;
	int i, peer_id;
	peer_id = ((Peer*)event->peer->data)->id;
	
	printf("#%i disconected.\n", peer_id);
	
	if (peers[peer_id].auth) {
		send_peer_disconnected(peer_id);
	}

	/* reset the peer's client information. */
	peers[peer_id].used = 0;
	peers[peer_id].auth = 0;
	event->peer->data = NULL;
}

static int valid(int c) {
	if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
	|| c == '-' || c == '_') {
		return 1;
	} else {
		return 0;
	}
}

static void handle_receive(ENetEvent* event) {
	int i, peer_id, c;
	peer_id = ((Peer*)event->peer->data)->id;
	dstring* s;
	
	printf("A packet of length %u was received from #%i on channel %i.\n",
		event->packet->dataLength,
		peer_id,
		event->channelID);
	
	if (event->channelID == 0) {
		/* auth packets */
		if (peers[peer_id].auth) {
			fprintf(stderr, "Received an auth packet from authorized peer!\n");
			enet_peer_disconnect(event->peer, 0);
			return;
		} else if (event->packet->dataLength > 31) {
			fprintf(stderr, "Nick too long!\n");
			enet_peer_disconnect(event->peer, 0);
			return;
		} else {
			for(i = 0; i < event->packet->dataLength; i++) {
				c = ((char*)(event->packet->data))[i];
				if (!valid(c)) {
					fprintf(stderr, "Invalid nick name!\n");
					enet_peer_disconnect(event->peer, 0);
					return;
				}
			}
		}
		
		memcpy(peers[peer_id].nick, event->packet->data, event->packet->dataLength);
		peers[peer_id].nick[event->packet->dataLength] = 0;
		
		printf("Peer #%i authorized as %s\n", peer_id, peers[peer_id].nick);
		
		/* check if this nick is unique */
		for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
			if (i == peer_id || !peers[i].used || !peers[i].auth) {
				continue;
			}
			
			if (strcmp(peers[peer_id].nick, peers[i].nick) == 0) {
				fprintf(stderr, "Nickname already in use\n");
				send_error(peer_id, "Nickname already in use");
				enet_peer_disconnect(event->peer, 0);
				return;
			}
		}
		
		peers[peer_id].auth = 1;
		send_auth(peer_id);
	} else if (event->channelID == 1) {
		/* system packets */
	} else if (event->channelID == 2) {
		/* audio packets */
		if (!peers[peer_id].auth) {
			fprintf(stderr, "Client #%i tried to send audio without autorizing\n", peer_id);
			return;
		}
		send_audio(peer_id, event->packet->data, event->packet->dataLength);
	} else if (event->channelID == 3) {
		/* errors */
	}


	/* Clean up the packet now that we're done using it. */
}



int main(int argc, char* argv[]) {
	/* host address */
	ENetAddress address;
	/* event */
	ENetEvent event;
	/* temporary buffer for string operations */
	char buff[1024];

	int i;

	/* initialize peers */
	memset(peers, 0, sizeof(Peer) * SVCSERVER_MAX_CLIENTS);

	if (enet_initialize () != 0) {
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return 1;
	}
	
	/* resolve server host */
	if (enet_address_set_host(&address, SERVER_HOST)) {
		fprintf(stderr, "Failed to resolve %s\n", SERVER_HOST);
		return 1;
	}
	address.port = SERVER_PORT;

	/* create server host */
	server = enet_host_create(&address, SVCSERVER_MAX_CLIENTS, SVCSERVER_MAX_CHANNELS, 0, 0);
	if (server == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet server host on %s:%i.\n",
					SERVER_HOST, address.port);
		return 1;
	}
	
	printf("Starting server at %s:%i\n", SERVER_HOST, address.port);

	/* main loop */
	while(1) {
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_CONNECT:
				handle_connection(&event);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				handle_receive(&event);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				handle_disconnect(&event);
				break;
			}
		}

	}

    enet_host_destroy(server);
    enet_deinitialize();
}
