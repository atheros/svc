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

#define SERVER_HOST		"0.0.0.0"

typedef struct {
	int id; /* index, stored in peer->data */
	int used;
	int auth; /* is client authorized */
	char nick[32]; /* nickname */
	ENetPeer* peer;
} Peer;



typedef enum {
	PACKET_AUTH;
};



static Peer peers[SVCSERVER_MAX_CLIENTS];
static ENetHost* server;








static void send_new_client_connected(
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (i == *(int*)(event.peer->data) || peers[i].used == 0) continue;
		ENetPacket * packet = enet_packet_create(buff, strlen(buff) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
		enet_peer_send(peers[i].peer, 0, packet);
	}
}



static void handle_connection(ENetEvent* event) {
	int i;
	for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
		if (!peers[i].used) {
			peers[i].id = i;
			peers[i].peer = event->peer;
			peers[i].used = 1;
			peers[i].auth = 0;
			event->peer->data = &peers[i];
			break;
		}
	}
	
	/* log a nice message */
	enet_address_get_host_ip(event->peer, buff, 1024);
	printf("Client #%i connected from %s:%u.", buff, event->peer->address.port);
	
	enet_host_flush(server);
}



static void handle_disconnect(ENetEvent* event) {
	ENetPacket* packet;
	int i, peer_id;
	peer_id = ((Peer*)event->peer->data)->id;
	
	printf("#%i disconected.\n", peer_id);
	if (peers[peer_id].auth) {
		/*
		for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
			if (client_id == i|| peers[i].used == 0) {
				continue;
			}
			
		}
		*/
	}

	/* reset the peer's client information. */
	peers[peer_id].used = 0;
	peers[peer_id].auth = 0;
	event.peer->data = NULL;
}



static void handle_receive(ENetEvent* event) {
	int i, peer_id;
	peer_id = ((Peer*)event->peer->data)->id;
	
	printf("A packet of length %u was received from #%s on channel %i.\n",
		event.packet -> dataLength,
		peer_id,
		event.channelID);
	
	if (event->channelID == 0) {
		/* auth packets */
		if (peers[peer_id].auth) {
			printf("Received an auth packet from authorized peer!\n");
			enet_diconnect(event->peer);
			return;
		} else if (event.packet->dataLength > 31) {
			printf("Nick too long!\n");
			enet_disconnect(event->peer);
			return;
		}
		
		memcpy(peers[peer_id].nick, event->packet->data, event.packet->dataLength);
		peers[peer_id].nick[event->packet->dataLength] = 0;
		
		/* check if this nick is unique */
		for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
			if (i == peer_id || peers[i].used == 0 || peers[i].auth == 0) {
				continue;
			}
			
			if (strcmp(peers[peer_id].nick, peers[i].nick) == 0) {
				send_error(peer_id, "Nickname already used");
				enet_disconnect(event->peer);
				return;
			}
		}
		
		send_auth(peer_id);
	} else if (event->channelID == 1) {
		/* system packets */
	} else if (event->channelID == 2) {
		/* audio packets */
	} else if (event->channelID == 3) {
		/* errors */
	}


	/* Clean up the packet now that we're done using it. */
	enet_packet_destroy (event.packet);
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
	address.port = SVCSERVER_PORT;

	/* create server host */
	server = enet_host_create(&address, SVCSERVER_MAX_CLIENTS, SVCSERVER_MAX_CHANNELS, 0, 0);
	if (server == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet server host on %s:%i.\n",
					SERVER_HOST, address.port);
		return 1;
	}

	/* main loop */
	while(1) {
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_CONNECT:
				handle_connection(&event);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				handle_receive(&event);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				handle_disconnect((Peer*)event.peer->data);
				break;
			}
		}

	}

    enet_host_destroy(server);
    enet_deinitialize();
}
