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



typedef struct {
	int id; // index, stored in peer->data
	int used;
	ENetPeer* peer;
	int muted;
	int defen;
	int bps;
} Peer;

Peer peers[SVCSERVER_MAX_CLIENTS];

int main(int argc, char* argv[]) {
    ENetAddress address;
    ENetHost * server;
    ENetEvent event;
    char buff[1024];
    int i;

    memset(peers, 0, sizeof(Peer) * SVCSERVER_MAX_CLIENTS);

	if (enet_initialize () != 0) {
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return 1;
	}

    address.host = ENET_HOST_ANY;
    address.port = SVCSERVER_PORT;

    server = enet_host_create(&address, SVCSERVER_MAX_CLIENTS, SVCSERVER_MAX_CHANNELS, 0, 0);
	if (server == NULL) {
		fprintf (stderr, "An error occurred while trying to create an ENet server host.\n");
		return 1;
	}

	while(1) {
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_CONNECT:
				for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
					if (peers[i].used == 0) {
						peers[i].id = i;
						peers[i].peer = event.peer;
						peers[i].used = 1;
						event.peer->data = &peers[i].id;
						break;
					}
				}

				sprintf(buff, "Client #%i connected from %i.%i.%i.%i:%u.", *(int*)(event.peer->data),
						(event.peer->address.host) & 0xff,
						(event.peer->address.host >> 8) & 0xff,
						(event.peer->address.host >> 16) & 0xff,
						(event.peer->address.host >> 24) & 0xff,
						event.peer->address.port);
				fprintf(stdout, "%s\n", buff);

				for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
					if (i == *(int*)(event.peer->data) || peers[i].used == 0) continue;
					ENetPacket * packet = enet_packet_create(buff, strlen(buff) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
					enet_peer_send(peers[i].peer, 0, packet);
				}

				enet_host_flush(server);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
				event.packet -> dataLength,
				event.packet -> data,
				event.peer -> data,
				event.channelID);

				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy (event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				sprintf(buff, "#%i disconected.", *(int*)event.peer->data);
				printf("%s\n", buff);


				for(i = 0; i < SVCSERVER_MAX_CLIENTS; i++) {
					if (i == *(int*)(event.peer->data) || peers[i].used == 0) continue;
					ENetPacket * packet = enet_packet_create(buff, strlen(buff) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
					enet_peer_send(peers[i].peer, 0, packet);
				}

				/* Reset the peer's client information. */
				peers[*(int*)event.peer->data].used = 0;
				event.peer -> data = NULL;
			}
		}

	}

    enet_host_destroy(server);
    enet_deinitialize();
}
