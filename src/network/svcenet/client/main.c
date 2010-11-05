#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>


#include "enetcheck.h"
#include "config.h"
#include "libsvc.h"


typedef struct {
	int id; /* index, stored in peer->data */
	char nick[32]; /* nickname */
	int used;
	peer_t* peer;
} Peer;

static Peer* peers = NULL;
static ENetPeer* serverPeer = NULL;
static ENetHost* client = NULL;
static int got_auth = 0;
static int my_id = 0;
static int max_clients = 0;



static void send_auth(ENetPeer* peer, const char* nick) {
	ENetPacket* packet;
	packet = enet_packet_create(nick, strlen(nick),  ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

static void handle_peer_list(unsigned char* p, unsigned int length) {
	int pc, x, len, i;
	int last = 0;
	int peer_id;
	
	x = 2;
	for(pc = 0; pc < p[1]; pc++) {
		peer_id = p[x++];
		len = p[x++];
		for(i = last; i < peer_id; i++) {
			fflush(stdout);
			if (peers[i].used) {
				peers[i].used = 0;
				svc_peer_leave(peers[i].peer);
			}
		}
		
		if (!peers[peer_id].used) {
			peers[peer_id].used = 1;
			peers[peer_id].peer = svc_peer_join();
		}
		
		memcpy(peers[peer_id].nick, p+x, len);
		peers[peer_id].nick[len] = 0;

		printf("Adding new peer #%i [%s]\n", peer_id, peers[peer_id].nick);
		if (peer_id == my_id) {
			printf("Heh, this is me!\n");
		}
		
		x+= len;
		last = peer_id;
	}
}

static void handle_peer_join(unsigned char* p, unsigned int length) {
	int peer_id = p[1];
	int len = p[2];
	
	if (peers[peer_id].used) {
		svc_peer_leave(peers[peer_id].peer);
	}
	
	peers[peer_id].used = 1;
	peers[peer_id].peer = svc_peer_join();
	memcpy(peers[peer_id].nick, p+3, len);
	peers[peer_id].nick[len] = 0;
	
	printf("Peer #%i [%s] joined\n", peer_id, peers[peer_id].nick);
}

static void handle_peer_left(unsigned char* p, unsigned int length) {
	int peer_id = p[1];
	
	if (peers[peer_id].used) {
		svc_peer_leave(peers[peer_id].peer);
		peers[peer_id].used = 0;
		printf("Peer #%i [%s] left\n", peer_id, peers[peer_id].nick);
	}
}


static void handle_audio(void* data, unsigned int len) {
	network_packet_t np;
	int peer_id = ((unsigned char*)data)[0];
	
	if (peers[peer_id].used) {

		np.data = (unsigned char*)data+3;
		np.data_len = len - 3;
		np.time = ((unsigned char*)data)[1]
			| ((unsigned char*)data)[2] << 8;
		
		/*printf("Got audio from %i, playing %i\n", peer_id, (int)np.time);*/
		svc_packet_recieve(&np, peers[peer_id].peer);
	} else {
		/*printf("Got audio from %i, not playing\n", peer_id);*/
	}
	
}

static int handle_receive(ENetEvent* event) {
	char* buff;
	unsigned char* p;
	
	/*
	printf("A packet of length %u on channel %u.\n",
		event->packet->dataLength,
		event->channelID);
	*/

	switch(event->channelID) {
	case 0:
		if (got_auth) {
			fprintf(stderr, "Second authorization received.\n");
			break;
		}
		/* we got auth */
		/* FIXME: use a temp buffer */
		p = (unsigned char*)(event->packet->data);
		my_id = p[0];
		max_clients = p[1];
		got_auth = 1;

		/*printf("Authorization accepted (my id %i, max clients %i).\n", my_id, max_clients);*/

		peers = (Peer*)malloc(sizeof(Peer) * max_clients);
		memset(peers, 0, sizeof(Peer) * max_clients);
		break;
		
	case 1:
		/* system packets */
		p = (unsigned char*)(event->packet->data);
		/*printf("Got system message of type %i\n", p[0]);*/
		
		switch (p[0]) {
		case SYSPACKAGE_LIST:
			handle_peer_list(p, event->packet->dataLength);
			break;
		case SYSPACKAGE_JOIN:
			handle_peer_join(p, event->packet->dataLength);
			break;
		case SYSPACKAGE_LEFT:
			handle_peer_left(p, event->packet->dataLength);
			break;
		default:
			fprintf(stderr, "Invalid system message of type %i\n", p[0]);
			break;
		}
		break;
		
	case 2:
		/* audio packets */
		handle_audio(event->packet->data, event->packet->dataLength);
		break;

	case 3:
		/* errors */
		buff = (char*)malloc(event->packet->dataLength+1);
		memcpy(buff, event->packet->data, event->packet->dataLength);
		buff[event->packet->dataLength] = 0;
		fprintf(stderr, "Received error from server: %s\n", buff);
		free(buff);
		enet_peer_reset(serverPeer);
		break;
	}


	/* Clean up the packet now that we're done using it. */
	enet_packet_destroy(event->packet);
	
	
	return 1;
}


void send_callback(network_packet_t* packet){
	ENetPacket* p;
	
	if (got_auth) {
		p = enet_packet_create(NULL, packet->data_len+2, ENET_PACKET_FLAG_UNSEQUENCED);
		p->data[0] = packet->time & 0xFF;
		p->data[1] = (packet->time >> 8) & 0xFF;
		
		memcpy(p->data + 2, packet->data, packet->data_len);
		
		enet_peer_send(serverPeer, 2, p);
		enet_host_flush(client);
	}
}


int main(int argc, char* argv[]) {
	ENetAddress address;
	ENetEvent event;

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
	
	svc_init(send_callback);

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
	serverPeer = enet_host_connect(client, &address, SVCSERVER_MAX_CHANNELS, 0);
	if (serverPeer == NULL) {
		fprintf(stderr, "No available peers for initialing an ENet connection.\n");
		return 1;
	}

	/* wait up to 5sec for the connection to succeed */
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		fprintf(stdout, "Connection to %s:%s succeeded.\n", argv[2], argv[3]);
		send_auth(serverPeer, argv[1]);
	} else {
		enet_peer_reset(serverPeer);
		fprintf(stderr, "Connection to %s:%s failed.\n", argv[2], argv[3]);
		return 1;
	}


	while(!done) {
		while (!done && enet_host_service(client, &event, 1000) > 0) {
			switch (event.type)	{
			case ENET_EVENT_TYPE_RECEIVE:
				if (!handle_receive(&event)) {
					done = 1;
				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf ("Server disconected.\nAborting!\n");
				done = 1;
				break;
			}
		}
	}
	
	if (peers) {
		free(peers);
	}
	svc_close();
	enet_host_destroy(client);
	enet_deinitialize();
}