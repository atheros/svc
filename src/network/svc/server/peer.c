#include <stdlib.h>
#include "peer.h"


PeerList* peers_alloc(unsigned int max) {
	PeerList* list;

	if (max == 0) {
		return NULL;
	}

	list = (PeerList*)malloc(sizeof(PeerList));
	if (list == NULL) {
		return NULL;
	}

	list->size = max;
	list->count = 0;
	list->peers = (PeerInfo*)malloc(sizeof(PeerInfo) * max);

	memset(list->peers, 0, (sizeof(PeerInfo) * max));

	return list;
}

void peers_free(PeerList* peers) {
	unsigned int i;
	for(i = 0; i < peers->size; i++) {
		if (peers->peers[i].nick) {
			dfree(peers->peers[i].nick);
		}

		if (peers->peers[i].key) {
			dfree(peers->peers[i].key);
		}
	}

	free(peers->peers);
	free(peers);
}


PeerId peers_add(PeerList* peers) {
	PeerId i;

	for(i = 0; i < peers->size; i++) {
		if (peers->peers[i].used == 0) {
			peers->peers[i].used = 1;
			peers->peers[i].peer_id = i;
			peers->peers[i].initialized = 0;
			peers->count++;
			return i;
		}
	}

	/* erm... what just happend? */
	return PEER_NULL;
}

void peers_remove(PeerList* peers, PeerId peer) {
	/* FIXME: an assert would be nice to check if peer is in use or is in correct range */
	peers->peers[peer].used = 0;
	if (peers->peers[peer].nick) {
		dfree(peers->peers[peer].nick);
	}

	if (peers->peers[peer].key) {
		dfree(peers->peers[peer].key);
	}

	peers->count--;
}
