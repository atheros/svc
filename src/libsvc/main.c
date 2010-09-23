#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "enetcheck.h"
#include "config.h"


int main(int argc, char* argv[]) {
    ENetHost* client;
    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    int done = 0;

    /* check if we have host and port set */
    if (argc != 3) {
    	fprintf(stderr, "usage: svccd <server_host> <server_port>\n");
    	return 1;
    }

	if (enet_initialize () != 0) {
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return 1;
	}

	/* create client */
    client = enet_host_create(NULL, 2, SVCSERVER_MAX_CHANNELS, 0, 0);
	if (client == NULL) {
		fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
		return 1;
	}

	/* set address to connect to */
	enet_address_set_host(&address, argv[1]);
	address.port = atoi(argv[2]);

	/* connect */
	peer = enet_host_connect(client, &address, SVCSERVER_MAX_CHANNELS, 0);
	if (peer == NULL) {
		fprintf(stderr, "No available peers for initialing an ENet connection.\n");
		return 1;
	}

	/* wait up to 5sec for the connection to succeed */
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		fprintf(stdout, "Connection to %s:%s succeeded.\n", argv[1], argv[2]);
	} else {
		enet_peer_reset (peer);
		fprintf(stderr, "Connection to %s:%s.\n", argv[1], argv[2]);
	}


	while(!done) {
		while (enet_host_service(client, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_RECEIVE:
				printf ("A packet of length %u {%s} on channel %u.\n",
				event.packet -> dataLength,
				event.packet -> data,
				event.channelID);

				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy (event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf ("server disconected.\n", event.peer -> data);
				/* Reset the peer's client information. */
				event.peer -> data = NULL;
				done = 1;
			}
		}
	}

    enet_host_destroy(client);
    enet_deinitialize();
}
