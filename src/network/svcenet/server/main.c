/*
 * main.c
 *
 *  Created on: 14-03-2011
 *      Author: przemek
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <enet/enet.h>
#include <sys/time.h>
#include "jim.h"

#include "enetcheck.h"
#include "config.h"
#include "dstr.h"
#include "dstrutils.h"

/*
 * CANNELS
 * 0 - binary protocol		- RELIABLE, IN-ORDER
 * 1 - text protocol		- RELIABLE,
 * 2 - audio protocol		- 0
 *
 *
 */


typedef struct _DataDictNode{
	struct _DataDictNode *prev, *next;

	int dirty;
	dstring* key;
	dstring* value;
} DataDictNode;

typedef struct {
	DataDictNode *front, *back;
} DataDict;

typedef struct {
	int				empty;
	unsigned int	id;
	ENetPeer*		peer;
	DataDict*		pub_data;
	DataDict*		priv_data;
} Peer;

static Peer* peers;
static int peers_count;
static int peers_size;
static int mainloop_done;
static DataDict* server_data;

static unsigned int audio_packet_length;
static void* audio_packet_data;

static unsigned long last_logic_time;


static DataDict* dd_new() {
	DataDict* dd = (DataDict*)malloc(sizeof(DataDict));
	dd->front = NULL;
	return dd;
}

static void dd_free(DataDict* dd) {
	DataDictNode* ddn;
	if (dd->front) {
		while(dd->front) {
			ddn = dd->front;
			dfree(ddn->key);
			dfree(ddn->value);
			dd->front = ddn->next;
			free(ddn);
		}
	}

	free(dd);
}

static void dd_set(DataDict* dd, const char* key, const char* value) {
	DataDictNode* ddn = dd->front;

	/* check existing nodes */
	if (ddn) {
		while (ddn) {
			if (dcmpcs(ddn->key, key) == 0) {
				dcpycs(ddn->value, value);
				ddn->dirty = 1;
				return;
			}
			ddn = ddn->next;
		}
	}

	/* new node */
	if (dd->front == NULL) {
		dd->front = dd->back = ddn = (DataDictNode*)malloc(sizeof(DataDictNode));
		ddn->next = NULL;
		ddn->prev = NULL;
	} else {
		ddn = (DataDictNode*)malloc(sizeof(DataDictNode));
		ddn->prev = dd->back;
		dd->back->next = ddn;
		ddn->prev->next = ddn;
	}

	ddn->dirty = 1;
	ddn->key = dfromcs(key);
	ddn->value = dfromcs(value);
}

static const char* dd_get(DataDict* dd, const char* key, int *len) {
	DataDictNode* node = dd->front;
	while (node) {
		if (dcmpcs(node->key, key) == 0) {
			if (len) *len = node->value->len;
			return node->value->data;
		}
		node = node->next;
	}
	if (len) *len = 0;
	return "";
}
/*
static const char* dd_remove(DataDict* dd, const char* key) {
	DataDictNode* node = dd->front;
	while (node) {
		if (dcmpcs(node->key, key) == 0) {
			if (node->prev) {
				node->prev->next = node->next;
			}
			if (node->next) {
				node->next->prev = node->prev;
			}
			if (node == dd->front) {
				dd->front = node->next;
			}
			if (node == dd->back) {
				dd->back = node->prev;
			}

			dfree(node->key);
			dfree(node->value);
			free(node);
		}
		node = node->next;
	}
}
*/

static int jim_get_int(Jim_Interp* interp, const char* name, int default_value) {
	Jim_Obj* obj;
	long val;

	obj = Jim_GetGlobalVariableStr(interp, name, 0);
	if (!obj) {
		return default_value;
	}

	if (Jim_GetLong(interp, obj, &val) == JIM_ERR) {
		return default_value;
	} else {
		return val;
	}
}

static const char* jim_get_str(Jim_Interp* interp, const char* name, const char* default_value) {
	Jim_Obj* obj;
	const char* val;

	obj = Jim_GetGlobalVariableStr(interp, name, 0);
	if (!obj) {
		return default_value;
	}

	val = Jim_GetString(obj, NULL);
	if (val == NULL) {
		return default_value;
	} else {
		return val;
	}
}

static int get_new_peer() {
	Peer* temp;
	int i, peer_id;

	if (peers_size == 0) {
		peers_size = 10;
		peers = (Peer*)malloc(sizeof(Peer) * peers_size);
		memset(peers, 0, sizeof(Peer) * peers_size);
		peers_count = 0;
		peer_id = 0;
		for(i = 0; i < peers_size; i++) {
			peers[i].empty = 1;
			peers[i].id = i;
		}
	} else if (peers_count == peers_size) {
		temp = (Peer*)malloc(sizeof(Peer) * (peers_size + 10));
		memcpy(temp, peers, sizeof(Peer) * peers_size);
		for(i = peers_size; i < peers_size+10; i++) {
			temp[i].empty = 1;
			temp[i].id = i;
		}
		peers_size+= 10;
		peer_id = peers_count;
	} else {
		for(i = 0; i < peers_size; i++) {
			if (peers[i].empty) {
				peer_id = i;
				break;
			}
		}
	}



	peers[peer_id].empty = 0;
	peers[peer_id].peer = NULL;
	peers[peer_id].priv_data = dd_new();
	peers[peer_id].pub_data = dd_new();

	peers_count++;
	return peer_id;
}

static void free_peer(unsigned int peer_id) {
	dd_free(peers[peer_id].priv_data);
	dd_free(peers[peer_id].pub_data);
	peers_count--;
	peers[peer_id].empty = 1;
}

static void free_peers() {
	unsigned int i;
	if (peers) {
		for(i = 0; i < peers_size; i++) {
			if (peers[i].empty) continue;

			enet_peer_disconnect_now(peers[i].peer, 0);
			dd_free(peers[i].priv_data);
			dd_free(peers[i].pub_data);
		}

		free(peers);
	}
}

/**
 * Returns a list of peers IDs connected to server.
 */
static int cmd_peers_list(Jim_Interp *interp, int argc, Jim_Obj *const *argv) {
	Jim_Obj *list, *item;
	int i;

	list = Jim_NewListObj(interp, NULL, 0);
	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty) continue;

		item = Jim_NewIntObj(interp, i);
		Jim_ListAppendElement(interp, list, item);
	}

	Jim_SetResult(interp, list);
	return JIM_OK;
}

static int cmd_peerset(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	long int peer_id;
	const char *key, *val, *type;
	int key_len, val_len;
	int priv;

	if (argc != 5) {
		Jim_SetResultString(interp, "peerset requires 4 arguments", -1);
		return JIM_ERR;
	}

	type = Jim_GetString(argv[1], NULL);
	if (type == NULL) {
		Jim_SetResultString(interp, "peerset argument 1 should be 'pub' or 'priv'", -1);
		return JIM_ERR;
	} else if (strcmp(type, "priv") == 0) {
		priv = 1;
	} else if (strcmp(type, "pub") == 0) {
		priv = 0;
	} else {
		Jim_SetResultString(interp, "peerset argument 1 should be 'pub' or 'priv'", -1);
		return JIM_ERR;
	}

	/* get peer ID */
	if (Jim_GetLong(interp, argv[2], &peer_id) == JIM_ERR) {
		Jim_SetResultString(interp, "peerset argument 2 should be an integer peer ID", -1);
		return JIM_ERR;
	}

	/* get key */
	key = Jim_GetString(argv[3], &key_len);
	if (key == NULL) {
		Jim_SetResultString(interp, "peerset argument 3 should be a string key", -1);
		return JIM_ERR;
	}

	/* get msg */
	val = Jim_GetString(argv[4], &val_len);
	if (val == NULL) {
		Jim_SetResultString(interp, "peerset argument 4 should be a string value", -1);
		return JIM_ERR;
	}

	/* get peer */
	if (peer_id >= 0 && peer_id < peers_size && !peers[peer_id].empty) {
		if (priv) {
			dd_set(peers[peer_id].priv_data, key, val);
		} else {
			dd_set(peers[peer_id].pub_data, key, val);
		}
		return JIM_OK;
	} else {
		Jim_SetResultString(interp, "peerset argument 2: peer doesn't exist ", -1);
		return JIM_ERR;
	}
}


static int cmd_peerget(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	long int peer_id;
	const char *key, *val, *type;
	int key_len, val_len;
	int priv;

	if (argc != 5) {
		Jim_SetResultString(interp, "peerget requires 3 arguments", -1);
		return JIM_ERR;
	}

	type = Jim_GetString(argv[1], NULL);
	if (type == NULL) {
		Jim_SetResultString(interp, "peerget argument 1 should be 'pub' or 'priv'", -1);
		return JIM_ERR;
	} else if (strcmp(type, "priv") == 0) {
		priv = 1;
	} else if (strcmp(type, "pub") == 0) {
		priv = 0;
	} else {
		Jim_SetResultString(interp, "peerget argument 1 should be 'pub' or 'priv'", -1);
		return JIM_ERR;
	}

	/* get peer ID */
	if (Jim_GetLong(interp, argv[2], &peer_id) == JIM_ERR) {
		Jim_SetResultString(interp, "peerget argument 2 should be an integer peer ID", -1);
		return JIM_ERR;
	}

	/* get key */
	key = Jim_GetString(argv[3], &key_len);
	if (key == NULL) {
		Jim_SetResultString(interp, "peerget argument 3 should be a string key", -1);
		return JIM_ERR;
	}

	/* get peer */
	if (peer_id >= 0 && peer_id < peers_size && !peers[peer_id].empty) {
		if (priv) {
			val = dd_get(peers[peer_id].priv_data, key, &val_len);
		} else {
			val = dd_get(peers[peer_id].pub_data, key, &val_len);
		}
		Jim_SetResultString(interp, val, val_len);
		return JIM_OK;
	} else {
		Jim_SetResultString(interp, "peerget argument 2: peer doesn't exist ", -1);
		return JIM_ERR;
	}
}


static int cmd_serverset(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	const char *key, *val;
	int key_len, val_len;

	if (argc != 3) {
		Jim_SetResultString(interp, "serverset requires 2 arguments", -1);
		return JIM_ERR;
	}

	/* get key */
	key = Jim_GetString(argv[1], &key_len);
	if (key == NULL) {
		Jim_SetResultString(interp, "serverset argument 1 should be a string key", -1);
		return JIM_ERR;
	}

	/* get msg */
	val = Jim_GetString(argv[2], &val_len);
	if (val == NULL) {
		Jim_SetResultString(interp, "serverset argument 2 should be a string value", -1);
		return JIM_ERR;
	}

	dd_set(server_data, key, val);
	return JIM_OK;
}


static int cmd_serverget(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	const char *key, *val;
	int key_len, val_len;

	if (argc != 2) {
		Jim_SetResultString(interp, "serverget requires 1 arguments", -1);
		return JIM_ERR;
	}

	/* get key */
	key = Jim_GetString(argv[1], &key_len);
	if (key == NULL) {
		Jim_SetResultString(interp, "serverget argument 1 should be a string key", -1);
		return JIM_ERR;
	}

	val = dd_get(server_data, key, &val_len);
	Jim_SetResultString(interp, val, val_len);
	return JIM_OK;
}


static int cmd_millitime(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	struct timeval tv;

	if (argc != 1) {
		Jim_SetResultString(interp, "millitime doesn't accept any arguments", -1);
		return JIM_ERR;
	}

	gettimeofday(&tv, NULL);
	Jim_SetResultInt(interp, (jim_wide)(tv.tv_sec) * (jim_wide)1000 + (jim_wide)tv.tv_usec / (jim_wide)1000);
	return JIM_OK;
}

static int cmd_sendaudio(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	long int peer_id;
	ENetPacket* packet;

	if (argc != 2) {
		Jim_SetResultString(interp, "sendaudio requires exactly 1 argument", -1);
		return JIM_ERR;
	}

	if (Jim_GetLong(interp, argv[1], &peer_id) == JIM_ERR) {
		Jim_SetResultString(interp, "sendaudio argument 1 should be an integer peer ID", -1);
		return JIM_ERR;
	}

	if (peer_id >= 0 && peer_id < peers_size && !peers[peer_id].empty) {
		if (audio_packet_data == NULL) {
			Jim_SetResultString(interp, "sendaudio: no audio packet available", -1);
			return JIM_ERR;
		}

		packet = enet_packet_create(audio_packet_data, audio_packet_length, ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_NO_ALLOCATE);
		enet_peer_send(peers[peer_id].peer, 1, packet);
		return JIM_OK;
	} else {
		Jim_SetResultString(interp, "sendaudio argument 2: peer doesn't exist", -1);
		return JIM_ERR;
	}
}

static int cmd_sendmsg(Jim_Interp* interp, int argc, Jim_Obj *const *argv) {
	long int peer_id;
	int len;
	const char* msg;
	ENetPacket* packet;

	if (argc != 3) {
		Jim_SetResultString(interp, "sendmsg requires 2 argument", -1);
		return JIM_ERR;
	}

	if (Jim_GetLong(interp, argv[1], &peer_id) == JIM_ERR) {
		Jim_SetResultString(interp, "sendmsg argument 1 should be an integer peer ID", -1);
		return JIM_ERR;
	}

	msg = Jim_GetString(argv[2], &len);
	if (msg == NULL) {
		Jim_SetResultString(interp, "sendmsg argument 2 should be a message to send", -1);
		return JIM_ERR;
	}

	if (peer_id >= 0 && peer_id < peers_size && !peers[peer_id].empty) {
		packet = enet_packet_create(msg, len, ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[peer_id].peer, 0, packet);
		return JIM_OK;
	} else {
		Jim_SetResultString(interp, "sendmsg argument 1: peer doesn't exist", -1);
		return JIM_ERR;
	}
}



void handle_peer_connect(Jim_Interp* interp, ENetHost* host, ENetEvent* event) {
	dstring *packet_buff, *escape_buff;
	unsigned int i, peer_id = get_new_peer();
	ENetPacket* packet;
	DataDictNode* ddn;
	long int val;
	char buff[1024];


	peers[peer_id].peer = event->peer;
	event->peer->data = &(peers[peer_id].id);

	sprintf(buff, "on_peer_connect %i", peer_id);

	if (Jim_Eval(interp, buff) == JIM_ERR) {
		fprintf(stderr, "on_peer_connect failed: %s\n", Jim_GetString(interp->result, NULL));
		enet_peer_disconnect(event->peer, 0);
		free_peer(peer_id);
		return;
	}

	if (Jim_GetLong(interp, interp->result, &val) == JIM_ERR) {
		fprintf(stderr, "on_peer_connect failed: it didn't return an integer value\n");
		enet_peer_disconnect(event->peer, 0);
		free_peer(peer_id);
		return;
	}

	if (val == 0) {
		fprintf(stderr, "on_peer_connect failed: %s\n", Jim_GetString(interp->result, NULL));
		enet_peer_disconnect(event->peer, 0);
		free_peer(peer_id);
		return;
	}

	/* add server config to the packet */
	packet_buff = dnew();
	ddn = server_data->front;
	while(ddn) {
		dcatcs(packet_buff, "SSET ");
		/* encode key */
		escape_buff = dstrlex_escape(ddn->key);
		dcat(packet_buff, escape_buff);
		dfree(escape_buff);
		dcatcs(packet_buff, " ");
		/* encode value */
		escape_buff = dstrlex_escape(ddn->value);
		dcat(packet_buff, escape_buff);
		dfree(escape_buff);
		dcatcs(packet_buff, "\n");

		ddn = ddn->next;
	}

	/* we need to broadcast him pub data from every peer */
	sprintf(buff, "%i\n", peer_id);
	dcpycs(packet_buff, "YARE ");
	dcatcs(packet_buff, buff);

	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty) continue;
		/* add the peer (PADD) */
		sprintf(buff, "%i", peer_id);
		dcatcs(packet_buff, "PADD ");
		dcatcs(packet_buff, buff);
		dcatcs(packet_buff, "\n");

		/* set all public keys */
		ddn = peers[i].pub_data->front;
		while (ddn) {
			dcatcs(packet_buff, "PSET ");
			dcatcs(packet_buff, buff);
			dcatcs(packet_buff, " ");
			/* encode key */
			escape_buff = dstrlex_escape(ddn->key);
			dcat(packet_buff, escape_buff);
			dfree(escape_buff);
			dcatcs(packet_buff, " ");
			/* encode value */
			escape_buff = dstrlex_escape(ddn->value);
			dcat(packet_buff, escape_buff);
			dfree(escape_buff);
			dcatcs(packet_buff, "\n");

			ddn = ddn->next;
		}
	}

	/* send the packet */
	packet = enet_packet_create(packet_buff->data, packet_buff->len, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
	enet_peer_send(peers[peer_id].peer, 0, packet);
	enet_host_flush(host);

	/* say hello to other peers */
	dcpycs(packet_buff, "PADD ");
	sprintf(buff, "%i", peer_id);
	dcatcs(packet_buff, "\n");

	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty || peer_id == i) continue;
		packet = enet_packet_create(packet_buff->data, packet_buff->len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[peer_id].peer, 0, packet);
	}

	dfree(packet_buff);
}


static void handle_peer_disconnect(Jim_Interp* interp, ENetEvent* event) {
	ENetPacket* packet;
	char buff[1024];
	unsigned int i, len, peer_id = *(unsigned int*)event->peer->data;

	sprintf(buff, "on_peer_disconnect %i", peer_id);
	if (Jim_Eval(interp, buff) == JIM_ERR) {
		fprintf(stderr, "on_peer_disconnect failed: %s\n", Jim_GetString(interp->result, NULL));
	}

	enet_peer_disconnect(event->peer, 0);

	len = sprintf(buff, "PDEL %i", peer_id);
	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty || i == peer_id) continue;
		packet = enet_packet_create(buff, len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[i].peer, 0, packet);
	}

	free_peer(peer_id);
}

static int decode_audio_packet(unsigned int peer_id, ENetPacket* packet) {
	enet_uint32 src, len;

	if (packet->dataLength <= 8) {
		fprintf(stderr, "Audio packet from #%i is too small (%i bytes)\n", peer_id, (int)packet->dataLength);
		return 1;
	}

	src = ENET_NET_TO_HOST_32(*(enet_uint32*)packet->data + 0);
	len = ENET_NET_TO_HOST_32(*(enet_uint32*)packet->data + 4);
	if (src != peer_id) {
		fprintf(stderr, "Audio packet from #%i is said to be from #%i!\n", peer_id, src);
		return 1;
	}

	if (len != packet->dataLength - 8) {
		fprintf(stderr, "Audio packet from #%i is expected to be %i bytes long, but only %i bytes are available!\n",
				peer_id, len, (int)packet->dataLength - 8);
		return 1;
	}

	audio_packet_data = packet->data;
	audio_packet_length = packet->dataLength;

	return 0;
}

static void handle_packet_input_audio(Jim_Interp* interp, ENetHost* host, ENetEvent* event) {
	char buff[1024];
	unsigned int peer_id = *(unsigned int*)event->peer->data;

	/* decode audio packet */
	if (decode_audio_packet(peer_id, event->packet)) {
		enet_packet_destroy(event->packet);
		return;
	}

	/* run audio packet handler */
	sprintf(buff, "on_audio_packet %i", peer_id);
	if (Jim_Eval(interp, buff) == JIM_ERR) {
		fprintf(stderr, "on_audio_packet failed: %s\n", Jim_GetString(interp->result, NULL));
	}

	enet_host_flush(host);
	enet_packet_destroy(event->packet);
	audio_packet_data = NULL;
}

static void handle_packet_input_command(Jim_Interp* interp, ENetHost* host, ENetEvent* event) {
	dstring *src;
	dstrlist *dargs;
	dstrnode *node;
	unsigned int peer_id = *(unsigned int*)event->peer->data;

	int code;
	Jim_Obj *args[2];


	src = dfrommem(event->packet->data, event->packet->dataLength);
	dargs = dstrlex_parse(src, &code);
	dfree(src);

	args[0] = Jim_GetGlobalVariableStr(interp, "on_command", 0);
	args[1] = Jim_NewListObj(interp, NULL, 0);

	node = dargs->front;
	while(node) {
		Jim_ListAppendElement(interp, args[1], Jim_NewStringObj(interp, node->string->data, node->string->len));
		node = node->next;
	}
	dlist_free(dargs);

	if (Jim_EvalObjVector(interp, 2, args) == JIM_ERR) {
		fprintf(stderr, "Failed to execute commands from #%i\n", peer_id);
		return;
	}

	return;
}

static unsigned long milliseconds() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long)(tv.tv_sec) * (unsigned long)1000 + (unsigned long)tv.tv_usec / (unsigned long)1000;
}

static void send_modified_data() {
	dstring *packet_buff, *escape_buff;
	char buff[1024];
	DataDictNode* ddn;
	unsigned int i;
	ENetPacket* packet;


	/* add server config to the packet */
	packet_buff = dnew();
	ddn = server_data->front;
	while(ddn) {
		if (ddn->dirty) {
			ddn->dirty = 0;
			dcatcs(packet_buff, "SSET ");
			/* encode key */
			escape_buff = dstrlex_escape(ddn->key);
			dcat(packet_buff, escape_buff);
			dfree(escape_buff);
			dcatcs(packet_buff, " ");
			/* encode value */
			escape_buff = dstrlex_escape(ddn->value);
			dcat(packet_buff, escape_buff);
			dfree(escape_buff);
			dcatcs(packet_buff, "\n");
		}
		ddn = ddn->next;
	}

	/* we need to broadcast him pub data from every peer */
	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty) continue;
		sprintf(buff, "%i", i);
		/* set all public keys */
		ddn = peers[i].pub_data->front;
		while (ddn) {
			if (ddn->dirty) {
				ddn->dirty = 0;
				dcatcs(packet_buff, "PSET ");
				dcatcs(packet_buff, buff);
				dcatcs(packet_buff, " ");
				/* encode key */
				escape_buff = dstrlex_escape(ddn->key);
				dcat(packet_buff, escape_buff);
				dfree(escape_buff);
				dcatcs(packet_buff, " ");
				/* encode value */
				escape_buff = dstrlex_escape(ddn->value);
				dcat(packet_buff, escape_buff);
				dfree(escape_buff);
				dcatcs(packet_buff, "\n");
			}
			ddn = ddn->next;
		}
	}

	/* send the packet */
	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty) continue;
		packet = enet_packet_create(packet_buff->data, packet_buff->len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peers[i].peer, 0, packet);
	}
}

int main(int argc, char* argv[]) {
	int i;
	unsigned long delta;
	ENetHost* host;
	ENetAddress addr;
	ENetEvent event;
	int code;
	Jim_Interp* jim;
	char buff[100];

	/* initialize variables */
	peers = NULL;
	peers_count = 0;
	peers_size = 0;
	audio_packet_data = NULL;


	/* initialize enet */
	if (enet_initialize()) {
		fprintf(stderr, "Failed to initialize ENet\n");
		return 1;
	}

	/* init jim */
	jim = Jim_CreateInterp();
	Jim_RegisterCoreCommands(jim);
	Jim_CreateCommand(jim, "peers_list",	cmd_peers_list, NULL, NULL);
	Jim_CreateCommand(jim, "peerset",		cmd_peerset, NULL, NULL);
	Jim_CreateCommand(jim, "peerget",		cmd_peerget, NULL, NULL);
	Jim_CreateCommand(jim, "serverset",		cmd_serverset, NULL, NULL);
	Jim_CreateCommand(jim, "serverget",		cmd_serverget, NULL, NULL);
	Jim_CreateCommand(jim, "millitime",		cmd_millitime, NULL, NULL);
	Jim_CreateCommand(jim, "sendaudio",		cmd_sendaudio, NULL, NULL);
	Jim_CreateCommand(jim, "sendmsg", 		cmd_sendmsg, NULL, NULL);

	/* run all scripts */
	for(i = 1; i < argc; i++) {
		printf("Executing jim script %s...", argv[i]);
		if (Jim_EvalFile(jim, argv[i]) == JIM_ERR) {
			fprintf(stderr, "Jim script %s raised an error: %s\n", argv[i], Jim_GetString(Jim_GetResult(jim), NULL));
			Jim_FreeInterp(jim);
			enet_deinitialize();
			return 1;
		}
	}


	/* init server host */
	if (enet_address_set_host(&addr, jim_get_str(jim, "SERVER_HOST", "0.0.0.0"))) {
		fprintf(stderr, "Can't resolve host %s\n", jim_get_str(jim, "SERVER_HOST", "0.0.0.0"));
		return 0;
	}

	addr.port = jim_get_int(jim, "SERVER_PORT", 49001);
	host = enet_host_create(&addr, jim_get_int(jim, "MAX_PEERS", 256), 3, 0, 0);
	if (!host) {
		fprintf(stderr, "Can't create server socket on %s:%i\n",
				jim_get_str(jim, "SERVER_HOST", "0.0.0.0"),
				(int)addr.port
		);
		return 0;
	}


	/* main loop */
	mainloop_done = 0;
	last_logic_time = milliseconds();
	while(!mainloop_done) {
		/* read event */
		code = enet_host_service(host, &event, 50);
		if (code > 0) while (1) {
			/* we have an event */
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				handle_peer_connect(jim, host, &event);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				handle_peer_disconnect(jim, &event);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				if (event.channelID == 0) {
					handle_packet_input_command(jim, host, &event);
				} else if (event.channelID == 1) {
					handle_packet_input_audio(jim, host, &event);
				}
				break;

			case ENET_EVENT_TYPE_NONE:
				fprintf(stderr, "Unexpected event ENET_EVENT_TYPE_NONE!\n");
				break;
			}

			/* get next event */
			code = enet_host_service(host, &event, 0);
			if (code <= 0) {
				break;
			}
		} else if (code < 0) {
			fprintf(stderr, "enet_host_service returned with error code %i\n", code);
		}

		/* invoke logic */
		delta = milliseconds() - last_logic_time;
		sprintf(buff, "on_logic %lu", delta);
		if (Jim_Eval(jim, buff) == JIM_ERR) {
			fprintf(stderr, "on_logic raised an error: %s\n", Jim_GetString(Jim_GetResult(jim), NULL));
		}

		/* send modified public data */
		send_modified_data();
		enet_host_flush(host);
	}

	/* free peers */
	free_peers();

	/* close jim */
	Jim_FreeInterp(jim);

	/* close enet */
	enet_host_destroy(host);
	enet_deinitialize();

	return 0;
}
