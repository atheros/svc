#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "enetcheck.h"
#include "config.h"


typedef struct {
	int id; /* index, stored in peer->data */
	char nick[32]; /* nickname */
} Peer;


static void send_auth(ENetPeer* peer, const char* nick) {
	ENetPacket* packet;
	packet = enet_create_packet(nick, strlen(nick),  ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
	enet_packet_destroy(packet);
}


static void handle_receive(ENetEvent* event) {
	switch(event->channelID) {
		
	}
	printf ("A packet of length %u {%s} on channel %u.\n",
		event.packet -> dataLength,
		event.packet -> data,
		event.channelID);

	/* Clean up the packet now that we're done using it. */
	enet_packet_destroy (event.packet);
}

int main(int argc, char* argv[]) {
    ENetHost* client;
    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    int done = 0;

    /* check if we have host and port set */
    if (argc != 4) {
    	fprintf(stderr, "usage: svcclient <nickname> <server_host> <server_port>\n");
    	return 1;
    }

	if (enet_initialize () != 0) {
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return 1;
	}

	/* create client */
    client = enet_host_create(NULL, 2, SVCSERVER_MAX_CHANNELS, 0, 0);
	if (client == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
		return 1;
	}

	/* set address to connect to */
	if (enet_address_set_host(&address, argv[2])) {
		fprintf(stderr, "Failed to resolve %s\n", argv[2]);
		return 1;
	}
	address.port = atoi(argv[3]);


	/* connect */
	peer = enet_host_connect(client, &address, SVCSERVER_MAX_CHANNELS, 0);
	if (peer == NULL) {
		fprintf(stderr, "No available peers for initialing an ENet connection.\n");
		return 1;
	}

	/* wait up to 5sec for the connection to succeed */
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		fprintf(stdout, "Connection to %s:%s succeeded.\n", argv[2], argv[3]);
		send_auth(peer, argv[1]);
	} else {
		enet_peer_reset(peer);
		fprintf(stderr, "Connection to %s:%s.\n", argv[1], argv[2]);
		return 1;
	}


	while(!done) {
		while (!done && enet_host_service(client, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_RECEIVE:
				handle_receive(&event);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf ("Server disconected.\nAborting!");
				done = 1;
				break;
			}
		}
	}

    enet_host_destroy(client);
    enet_deinitialize();
}
