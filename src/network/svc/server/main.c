#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "dstrutils.h"
#include "server.h"


static struct option options[] = {
		{"bind",		required_argument,	NULL, 'b'},
		{"help",		no_argument,		NULL, 'h'},
		{"max-peers",	required_argument,	NULL, 'm'},
		{"port",		required_argument,	NULL, 'p'},
		{NULL, 0, NULL, 0}
};


static unsigned int max_peers;
static unsigned int port;
static char bindto[100];


static Server server;




void help(const char* app) {
	printf("usage: %s [options]\n", app);
	printf("Options:\n");
	printf("  -p,--port=PORT                    Specify server port number (default 49001).\n");
	printf("  -m,--max-peers=MAX                Specify maximum peers count (default 16).\n");
	printf("  -b,--bind=ADDR                    Bind to specified address (default 0.0.0.0).\n");
}


static int server_enet_init(ENetHost** host, ENetPeer** server_peer) {
	ENetAddress address;

	/* init enet */
	if (enet_initialize () != 0) {
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return 1;
	}

	/* set address to connect to */
	if (enet_address_set_host(&address, bindto)) {
		fprintf(stderr, "Failed to resolve %s\n", bindto);
		return 1;
	}
	address.port = port;

	/* create host */
	*host = enet_host_create(&address, max_peers+1, 10, 0, 0);
	if (*host == NULL) {
		fprintf(stderr, "An error occurred while trying to create an ENet server host on %s:%i.\n",
					bindto, address.port);
		return 1;
	}


	return 0;
}

static void server_enet_quit(ENetHost** host, ENetPeer** server_peer) {
	enet_peer_reset(*server_peer);
	enet_host_destroy(*host);
	enet_deinitialize();
}

static int server_enet_loop(Server* server, int timeout) {
	return 0;
}

int server_main() {
	/* init server */
	server.peers = peers_alloc(max_peers);
	server.channels = channels_alloc("ROOT");

	/* load channels */

	/* init server side enet stuff */
	if (server_enet_init(&(server.host), &(server.server_peer))) {
		return 1;
	}

	/* server loop (100 ms wait) */
	if (server_enet_loop(&server, 100)) {
		return 1;
	}

	/* send close to peers */
	/* close sockets */

	/* close server socket */
	server_enet_quit(&(server.host), &(server.server_peer));

	/* free server */
	peers_free(server.peers);
	channels_free(server.channels);

	return 0;
}

int main(int argc, char* argv[]) {
	int i, ptr;

	/* default values */
	max_peers = 16;
	port = 49001;
	strcpy(bindto, "0.0.0.0");

	while ((i = getopt_long(argc, argv, "b:hm:p:", options, &ptr)) >= 0) {
		switch (i) {
		case 'b':
			 strncpy(bindto, optarg, 100);
			 bindto[99] = 0;
			 break;

		case 'h':
			help(argv[0]);
			return 0;

		case 'm':
			max_peers = atoi(optarg);
			if (max_peers < 2) {
				fprintf(stderr, "Peers limit must be atleast 2.\n");
				return 1;
			} else if (max_peers > 65535) {
				fprintf(stderr, "Peers hard limit is 65535.\n");
				return 1;
			}
			break;

		case 'p':
			port = atoi(optarg);
			if (port == 0 || port > 65535) {
				fprintf(stderr, "Invalid port number %s\n", optarg);
				return 1;
			}
			break;

		}
	}

	return server_main();
}
