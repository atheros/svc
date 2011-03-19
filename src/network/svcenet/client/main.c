/*
Copyright 2011 Przemysław Grzywacz. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Przemysław Grzywacz ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Przemysław Grzywacz.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <enet/enet.h>
#include <signal.h>

#include "config.h"
#include "dstr.h"
#include "dstrutils.h"
#include "thread.h"

#include "libsvc.h"

typedef struct {
	int				connected;
	unsigned int	id;
	dsdict*			data;
	svc_peer_t*		peer;
	/* TODO: add audio pear */
} ClientPeer;

#define SVCECLIENT_STATE_DISCONNECTED	0
#define SVCECLIENT_STATE_CONNECTING		1
#define SVCECLIENT_STATE_CONNECTED		2


typedef struct {
	/**
	 *  Current client state:
	 * - SVCECLIENT_STATE_DISCONNECTED
	 * - SVCECLIENT_STATE_CONNECTING
	 * - SVCECLIENT_STATE_CONNECTED
	 */
	int				state;
	ENetHost*		host;
	ENetPeer*		client;

	int				main_done;

	thread_t		input_thread;
	mutex_t			input_lock;
	dstrlist*		input_queue;
	int				input_error;
	char*			input_buffer; /* thread private */
	size_t			input_buffer_size;

	mutex_t			network_lock;

	dsdict*			server_info;

	unsigned int	my_id;
	ClientPeer*		peers;
	int				peers_size;

	/* those flags are only local! */
	int				muted;
	int				deafen;
} ClientState;


/**
 * Main client state.
 */
static ClientState client;





static void sigint() {
	printf("Caught SIGINT, terminating\n");
	client.main_done = 1;
}




/**
 * audio callback.
 */
void send_callback(svc_network_packet_t* packet){
	enet_uint32 src;
	ENetPacket* epacket;

	unsigned char* data;

	/* get ID */
	mutex_lock(&client.network_lock);
	src = client.my_id;

	/* this client didn't get it's ID yet (this also handles connection state as my_id==-1 if not connected)
	 * or is muted. */
	if (src == -1 || client.muted) {
		mutex_unlock(&client.network_lock);
		return;
	}

	/* allocate data */
	data = (unsigned char*)malloc(packet->data_len+10);
	*(enet_uint32*)data = ENET_HOST_TO_NET_32(src);
	*(enet_uint32*)(data + 4) = ENET_HOST_TO_NET_32(packet->data_len+2);
	*(enet_uint16*)(data + 8) = ENET_HOST_TO_NET_16(packet->time);

	/* copy audio data */
	memcpy(data + 10, packet->data, packet->data_len);

	epacket = enet_packet_create(data, packet->data_len + 10, ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(client.client, 1, epacket);
	enet_host_flush(client.host);
	mutex_unlock(&client.network_lock);

	free(data);
}



/**
 * Input thread.
 *
 * It reads in a loop stdin input and passes it to a queue.
 * On error, it sets client.input_error and exits.
 */
static void *input_thread(void* dummy) {
	size_t len;
	char* s;
	while(1) {
		s = fgets(client.input_buffer, client.input_buffer_size, stdin);
		if (s) {
			len = strlen(s);
			if (len >= 1) {
				if (s[len-1] == '\n') {
					s[len-1] = 0;
					len--;
				}

				mutex_lock(&client.input_lock);
				dlist_addmem(client.input_queue, s, len);
				mutex_unlock(&client.input_lock);
			}
		} else {
			mutex_lock(&client.input_lock);
			client.input_error = 1;
			mutex_unlock(&client.input_lock);
			return NULL;
		}
	}
}

/**
 * Send state to UI.
 */
static void msg_state() {
	switch (client.state) {
	case SVCECLIENT_STATE_CONNECTED:	fprintf(stdout, ":STATE connected\n"); break;
	case SVCECLIENT_STATE_CONNECTING:	fprintf(stdout, ":STATE connecting\n"); break;
	case SVCECLIENT_STATE_DISCONNECTED:	fprintf(stdout, ":STATE disconnected\n"); break;
	default:
		fprintf(stdout, ":STATE unknown!!!\n");
		break;
	}
}

/**
 * Send muted state to UI.
 */
static void msg_muted() {
	fprintf(stdout, ":MUTED %i\n", client.muted);
}

/**
 * Send muted state to UI.
 */
static void msg_deafen() {
	fprintf(stdout, ":DEAFEN %i\n", client.deafen);
}

/**
 * Free resources associated with server state.
 */
static void free_server_info() {
	unsigned int i;
	/* free server info if available */
	if (client.server_info) {
		dsdict_free(client.server_info);
		client.server_info = NULL;
	}

	/* no mutex lock, lock outside of this function! */
	client.my_id = -1;

	/* free peers if available */
	if (client.peers) {
		for(i = 0; i < client.peers_size; i++) {
			if (client.peers[i].connected) {
				/* free resources available on connected peers */
				dsdict_free(client.peers[i].data);
				if (client.peers[i].peer) svc_peer_leave(client.peers[i].peer);
			}
		}

		/* free the peers structure */
		free(client.peers);
		client.peers = NULL;
		client.peers_size = 0;
	}
}

/**
 * Initialize client state.
 */
int init_client_state() {
	client.host = NULL;
	client.client = NULL;
	client.state = SVCECLIENT_STATE_DISCONNECTED;

	client.input_buffer_size = 1024*1024; /* or 1<<20? */
	client.input_buffer = (char*)malloc(client.input_buffer_size);
	client.input_queue = dlist_new();
	client.input_error = 0;

	mutex_create(&client.input_lock);
	thread_create(&client.input_thread, input_thread, NULL);

	mutex_create(&client.network_lock);

	client.server_info = NULL;
	client.peers = NULL;
	client.peers_size = 0;
	client.my_id = -1;

	return 0;
}

/**
 * Free client state.
 */
void quit_client_state() {
	int input_error;

	/* free enet structures */
	if (client.client) enet_peer_disconnect(client.client, 0);
	if (client.host) enet_host_destroy(client.host);


	/* get thread status */
	mutex_lock(&client.input_lock);
	input_error = client.input_error;
	mutex_unlock(&client.input_lock);

	/* cancel the input thread if needed */
	if (!input_error) {
		mutex_lock(&client.input_lock);
		thread_cancel(client.input_thread);
		mutex_unlock(&client.input_lock);
	}

	/* join a canceled thread */
	thread_join(client.input_thread);
	/* destroy the input lock */
	mutex_destroy(&client.input_lock);

	mutex_destroy(&client.network_lock);

	/* free other input resources */
	free(client.input_buffer);
	dlist_free(client.input_queue);

	free_server_info();
}

/**
 * COMMAND /connect host port
 *
 * Initiate connection to svceserver.
 */
static int cmd_connect(int argc, dstring** argv) {
	ENetAddress address;

	/* check if client is in correct state to connect */
	if (client.state != SVCECLIENT_STATE_DISCONNECTED) {
		if (client.state == SVCECLIENT_STATE_CONNECTED) {
			fprintf(stderr, "Already connected.\n");
		} else if (client.state == SVCECLIENT_STATE_CONNECTING) {
			fprintf(stderr, "Already trying to connect.\n");
		} else {
			fprintf(stderr, "Unknown client state.\n");
		}
		return 0;
	}

	if (argc != 3) {
		fprintf(stderr, "usage: /connect host port\n");
		return 0;
	}

	/* resolve host */
	address.port = atoi(argv[2]->data);
	if (enet_address_set_host(&address, argv[1]->data)) {
		fprintf(stderr, "Failed to resolve '%s'\n", argv[1]->data);
		return 0;
	}

	mutex_lock(&client.network_lock);
	/* create host */
	client.host = enet_host_create(NULL, 1, 2, 0, 0);
	if (client.host == NULL) {
		mutex_unlock(&client.network_lock);
		fprintf(stderr, "An error occurred while trying to create enet host.\n");
		return 0;
	}

	/* initiate connection */
	client.client = enet_host_connect(client.host, &address, 2, 0);
	if (client.client == NULL) {
		enet_host_destroy(client.host);
		mutex_unlock(&client.network_lock);
		fprintf(stderr, "No available peers to initiate a connection.\n");
		return 0;
	}

	mutex_unlock(&client.network_lock);

	client.state = SVCECLIENT_STATE_CONNECTING;
	msg_state();
	return 0;
}

/**
 * COMMAND /disconnect
 *
 * Terminate connection or abort a connection process.
 */
static int cmd_disconnect(int argc, dstring** argv) {
	mutex_lock(&client.network_lock);
	if (client.state == SVCECLIENT_STATE_CONNECTED) {
		free_server_info();
		enet_peer_disconnect(client.client, 0);
		enet_host_destroy(client.host);
		client.client = NULL;
		client.host = NULL;
		client.state = SVCECLIENT_STATE_DISCONNECTED;
		msg_state();
	} else if (client.state == SVCECLIENT_STATE_CONNECTING) {
		enet_peer_disconnect(client.client, 0);
		enet_host_destroy(client.host);
		client.client = NULL;
		client.host = NULL;
		client.state = SVCECLIENT_STATE_DISCONNECTED;
		msg_state();
	} else {
		fprintf(stderr, "Not connected.\n");
	}
	mutex_unlock(&client.network_lock);
	return 0;
}

/**
 * COMMAND /quit
 *
 * Terminate client.
 */
static int cmd_quit(int argc, dstring** argv) {
	return 1;
}

/**
 * COMMAND /mute flag
 *
 * Locally mute client.
 *
 * flag: 0 - muted, !=0 not muted
 */
static int cmd_mute(int argc, dstring** argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: /mute flag\n");
		return 0;
	}

	mutex_lock(&client.network_lock);
	client.muted = atoi(argv[1]->data) ? 1 : 0;
	mutex_unlock(&client.network_lock);

	msg_muted();
	return 0;
}

/**
 * COMMAND /deafen flag
 *
 * Locally mute client.
 *
 * flag: 0 - deafen, !=0 not deafen
 */
static int cmd_deafen(int argc, dstring** argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: /deafen flag\n");
		return 0;
	}
	mutex_lock(&client.network_lock);
	client.deafen = atoi(argv[1]->data) ? 1 : 0;
	mutex_unlock(&client.network_lock);

	msg_deafen();
	return 0;
}

/**
 * Handle input command.
 *
 * This function should return 0, even if the command failed.
 * Return of != 0 means the event loop will quit.
 */
static int handle_input_command(const dstring* string) {
	dstrlist* list;
	dstring** argv;
	unsigned int argc;
	int error;
	ENetPacket* packet;


	if (string->len == 0) {
		/* empty command */
		return 0;
	} else if (string->data[0] == '/') {
		/* local command */
		/* parse the command */
		list = dstrlex_parse(string, &error);
		if (list == NULL) {
			fprintf(stderr, "Failed to parse command string '%s': %s\n", string->data, dstrlex_errstr(error));
			return 0;
		}

		/* convert list to vector */
		argc = list->size;
		argv = dlist_tovector(list);
		dlist_free(list);

		/* select command to execute */
		if (dcmpcs(argv[0], "/connect") == 0) {
			error = cmd_connect(argc, argv);
		} else if (dcmpcs(argv[0], "/disconnect") == 0) {
			error = cmd_disconnect(argc, argv);
		} else if (dcmpcs(argv[0], "/quit") == 0) {
			error = cmd_quit(argc, argv);
		} else if (dcmpcs(argv[0], "/mute") == 0) {
			error = cmd_mute(argc, argv);
		} else if (dcmpcs(argv[0], "/deafen") == 0) {
			error = cmd_deafen(argc, argv);
		} else {
			fprintf(stderr, "Unknown command '%s'\n", argv[0]->data);
			error = 0;
		}

		dvec_free(argv);
		return error;
	} else if (client.state == SVCECLIENT_STATE_CONNECTED) {
		/* send to server if connected */
		mutex_lock(&client.network_lock);
		packet = enet_packet_create(string->data, string->len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(client.client, 0, packet);
		mutex_unlock(&client.network_lock);
	} else if (client.state == SVCECLIENT_STATE_CONNECTING) {
		/* server command but still connecting */
		fprintf(stderr, "Can't send command to server, still connecting.\n");
	} else {
		/* server command but not connected */
		fprintf(stderr, "Can't send command to server, not connected.\n");
	}


	return 0;
}

static int mempos(void* data, int ch, int start, int length) {
	char* str = (char*)data;
	while(start < length) {
		if (str[start] == ch) {
			return start;
		} else {
			start++;
		}
	}

	return -1;
}

static int decode_peer_id(const dstring* id) {
	return atoi(id->data);
}

static void scmd_sset(const dstring* cmd, int argc, dstring** argv) {
	if (argc != 3) {
		fprintf(stderr, "Invalid server command: SSET with %i arguments.\n", argc-1);
		return;
	}

	dsdict_set(client.server_info, argv[1]->data, argv[2]);
	fprintf(stdout, "%s\n", cmd->data);
}

static void scmd_yare(const dstring* cmd, int argc, dstring** argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid server command: YARE with %i arguments.\n", argc-1);
		return;
	}

	mutex_lock(&client.network_lock);
	client.my_id = decode_peer_id(argv[1]);
	mutex_unlock(&client.network_lock);

	fprintf(stdout, "%s\n", cmd->data);
}

static void scmd_padd(const dstring* cmd, int argc, dstring** argv) {
	ClientPeer* peers;
	unsigned int peer_id, i;
	if (argc != 2) {
		fprintf(stderr, "Invalid server command: PADD with %i arguments.\n", argc-1);
		return;
	}

	peer_id = decode_peer_id(argv[1]);

	if (client.peers_size == 0) {
		client.peers_size = peer_id + 1;
		client.peers = (ClientPeer*)malloc(sizeof(ClientPeer) * client.peers_size);
		for(i = 0; i < client.peers_size; i++) {
			client.peers[i].connected = 0;
			client.peers[i].data = NULL;
			client.peers[i].id = i;
			client.peers[i].peer = NULL;
		}
	} else if (client.peers_size <= peer_id) {
		/* enlarge peer vector */
		peers = (ClientPeer*)malloc(sizeof(ClientPeer) * (peer_id + 1));
		for (i = 0; i < client.peers_size; i++) {
			peers[i] = client.peers[i];
		}
		for(i = client.peers_size; i < (peer_id + 1); i++) {
			peers[i].connected = 0;
			peers[i].data = NULL;
			peers[i].id = i;
			peers[i].peer = NULL;
		}
		free(client.peers);
		client.peers = peers;
		client.peers_size = peer_id + 1;
	}

	client.peers[peer_id].connected = 1;
	client.peers[peer_id].data = dsdict_new();
	if (peer_id == client.my_id) {
		client.peers[peer_id].peer = NULL;
	} else {
		client.peers[peer_id].peer = svc_peer_join();
	}
	fprintf(stdout, "%s\n", cmd->data);
}

static void scmd_pdel(const dstring* cmd, int argc, dstring** argv) {
	unsigned int peer_id;

	if (argc != 2) {
		fprintf(stderr, "Invalid server command: PDEL with %i arguments.\n", argc-1);
		return;
	}

	peer_id = decode_peer_id(argv[1]);
	if (peer_id < 0 || peer_id >= client.peers_size) {
		fprintf(stderr, "Invalid peer_id %i in PDEL.\n", peer_id);
		return;
	}

	client.peers[peer_id].connected = 0;
	if (client.peers[peer_id].peer) svc_peer_leave(client.peers[peer_id].peer);
	dsdict_free(client.peers[peer_id].data);
	fprintf(stdout, "%s\n", cmd->data);
}


static void scmd_pset(const dstring* cmd, int argc, dstring** argv) {
	unsigned int peer_id;
	if (argc != 4) {
		fprintf(stderr, "Invalid server command: PSET with %i arguments.\n", argc-1);
		return;
	}

	peer_id = decode_peer_id(argv[1]);
	if (peer_id < 0 || peer_id >= client.peers_size) {
		fprintf(stderr, "Invalid peer_id %i in PSET.\n", peer_id);
		return;
	}

	dsdict_set(client.peers[peer_id].data, argv[2]->data, argv[3]);
	fprintf(stdout, "%s\n", cmd->data);
}

static void scmd_mesg(const dstring* cmd, int argc, dstring** argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid server command: MESG with %i arguments.\n", argc-1);
		return;
	}

	fprintf(stdout, "%s\n", cmd->data);
}

static void handle_server_decoded_msg(const dstring* string) {
	dstrlist* list;
	dstring** argv;
	int argc;
	int error;

	list = dstrlex_parse(string, &error);
	if (list == NULL) {
		fprintf(stderr, "Failed to parse string command: %s\n", dstrlex_errstr(error));
		return;
	}

	/* convert list to vector */
	argc = list->size;
	argv = dlist_tovector(list);
	dlist_free(list);

	if (dcmpcs(argv[0], "SSET") == 0) {
		scmd_sset(string, argc, argv);
	} else if (dcmpcs(argv[0], "YARE") == 0) {
		scmd_yare(string, argc, argv);
	} else if (dcmpcs(argv[0], "PADD") == 0) {
		scmd_padd(string, argc, argv);
	} else if (dcmpcs(argv[0], "PDEL") == 0) {
		scmd_pdel(string, argc, argv);
	} else if (dcmpcs(argv[0], "PSET") == 0) {
		scmd_pset(string, argc, argv);
	} else if (dcmpcs(argv[0], "MESG") == 0) {
		scmd_mesg(string, argc, argv);
	} else {
		fprintf(stderr, "Unrecognized command from server '%s'.\n", argv[0]->data);
	}
	dvec_free(argv);
}

static void handle_server_msg(ENetPacket* packet) {
	dstring* packet_str;
	int pos, prev;

	prev = 0;
	pos = mempos(packet->data, '\n', prev, packet->dataLength);
	while(pos>=0) {
		if (pos - prev > 0) {
			packet_str = dfrommem(packet->data + prev, pos - prev);
			handle_server_decoded_msg(packet_str);
			dfree(packet_str);
		}

		prev = pos + 1;
		pos = mempos(packet->data, '\n', prev, packet->dataLength);
	}
}

static void handle_server_audio(ENetPacket* packet) {
	enet_uint32 src, len;
	svc_network_packet_t np;

	/* ignore audio when locally muted */
	if (client.deafen) { /* FIXME: no lock, as deafen is only access from this thread (for now) */
		return;
	}

	/* decode header */
	src = ENET_NET_TO_HOST_32(*(enet_uint32*)(packet->data + 0));
	len = ENET_NET_TO_HOST_32(*(enet_uint32*)(packet->data + 4));
	if (src >= client.peers_size) {
		fprintf(stderr, "Invalid source %i in audio packet.\n", src);
		return;
	}

	/* make an audio packet */
	np.data = packet->data + 10;
	np.data_len = len - 2;
	np.time = ENET_NET_TO_HOST_16(*(enet_uint16*)(packet->data + 8));

	/* send audio to SVC */
	svc_packet_recieve(&np, client.peers[src].peer);
}


int main(int argc, char* argv[]) {
	dstring* input;
	int input_done;
	int code;
	ENetEvent event;

	/* initialize enet */
	if (enet_initialize()) {
		fprintf(stderr, "Failed to initialize ENet\n");
		return 1;
	}

	/* initialize client state */
	init_client_state();

	/* init SVC */
	svc_init(send_callback);


	/* main loop */
	input = dnew();
	client.main_done = 0;
	signal(SIGINT, sigint);
	while(!client.main_done) {
		/* enet event handler */
		if (client.state == SVCECLIENT_STATE_CONNECTED) {
			/* handle connection state */

			while (1) {
				mutex_lock(&client.network_lock);
				code = enet_host_service(client.host, &event, 0);
				mutex_unlock(&client.network_lock);
				if (code <= 0) break;

				/* handle event */
				switch(event.type) {
				case ENET_EVENT_TYPE_DISCONNECT:
					/* we got disconnected */
					mutex_lock(&client.network_lock);
					client.state = SVCECLIENT_STATE_DISCONNECTED;
					free_server_info();
					mutex_unlock(&client.network_lock);
					msg_state();
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					/* some data just arrived */
					if (event.channelID == 0) {
						handle_server_msg(event.packet);
					} else if (event.channelID == 1) {
						handle_server_audio(event.packet);
					} else {
						fprintf(stderr, "Received message from server on invalid channel %i.\n", event.channelID);
					}
					mutex_lock(&client.network_lock);
					enet_packet_destroy(event.packet);
					mutex_unlock(&client.network_lock);
					break;

				case ENET_EVENT_TYPE_CONNECT:
					/* no one will connect, ignore the bastard */
				default:
					break;
				}

			}
		} else if (client.state == SVCECLIENT_STATE_CONNECTING) {
			/* handle connecting state */
			mutex_lock(&client.network_lock);
			code = enet_host_service(client.host, &event, 0);
			mutex_unlock(&client.network_lock);

			if (code > 0) {
				if (event.type == ENET_EVENT_TYPE_CONNECT) {
					client.state = SVCECLIENT_STATE_CONNECTED;
					msg_state();
				} else {
					mutex_lock(&client.network_lock);
					client.state = SVCECLIENT_STATE_DISCONNECTED;
					enet_peer_disconnect(client.client, 0);
					client.client = NULL;
					enet_host_destroy(client.host);
					client.host = NULL;
					msg_state();
					mutex_unlock(&client.network_lock);
				}
			}
		} else {
			/* sleep for 5ms */
			usleep(5000);
		}

		/* play with the input queue */
		if (!client.main_done) {
			input_done = 0;
			while (!input_done) {
				/* get next input command */
				mutex_lock(&client.input_lock);
				if (client.input_queue->size) {
					dcpy(input, client.input_queue->front->string);
					dlist_erase(client.input_queue, client.input_queue->front);
				} else {
					input_done = 1;
				}
				mutex_unlock(&client.input_lock);

				/* quit input loop if queue is empty */
				if (input_done) {
					break;
				}

				/* handle commands */
				input_done = client.main_done = handle_input_command(input);
			}
		}

		/* check if input died for some reasons */
		mutex_lock(&client.input_lock);
		if (client.input_error) {
			/* if so, leave main loop */
			client.main_done = 1;
		}
		mutex_unlock(&client.input_lock);

	}

	fprintf(stdout, ":STATE exiting\n");
	dfree(input);

	svc_close();

	quit_client_state();

	enet_deinitialize();

	return 0;
}
