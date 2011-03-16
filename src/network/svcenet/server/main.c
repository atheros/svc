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
#include "jim.h"

#include "enetcheck.h"
#include "config.h"
#include "dstr.h"

typedef struct {
	int				empty;
	ENetPeer*		peer;
	int				identified;
	dstring*		name;
} Peer;



static Peer* peers = NULL;
static int peers_count;
static int peers_size;

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
	int i;

	if (peers_size == 0) {
		peers_size = 10;
		peers = (Peer*)malloc(sizeof(Peer) * peers_size);
		memset(peers, 0, sizeof(Peer) * peers_size);
		peers_count = 0;
	}

	if (peers_count == peers_size) {
		temp = (Peer*)malloc(sizeof(Peer) * (peers_size + 10));
		memcpy(temp, peers, sizeof(Peer) * peers_size);
		for(i = peers_size; i < peers_size+10; i++) {
			temp[i].empty = 0;
		}
		peers_size+= 10;
	}

	i = peers_count;
	peers_count++;
	return i;
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


int main(int argc, char* argv[]) {
	int i;
	ENetHost* host;
	ENetAddress addr;

	Jim_Interp* jim;

	if (enet_initialize()) {
		fprintf(stderr, "Failed to initialize ENet\n");
		return 1;
	}

	/* init jim */
	jim = Jim_CreateInterp();
	Jim_RegisterCoreCommands(jim);

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
	enet_address_set_host(&addr, jim_get_str(jim, "SERVER_HOST", "0.0.0.0"));
	addr.port = jim_get_int(jim, "SERVER_PORT", 49001);
	host = enet_host_create(&addr, jim_get_int(jim, "MAX_PEERS", 256), 3, 0, 0);






	/* free peers */
	for(i = 0; i < peers_size; i++) {
		if (peers[i].empty) continue;

		enet_peer_disconnect_now(peers[i].peer, 0);
	}

	/* close jim */
	Jim_FreeInterp(jim);

	/* close enet */
	enet_host_destroy(host);
	enet_deinitialize();

	return 0;
}
