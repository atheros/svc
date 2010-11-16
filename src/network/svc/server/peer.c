#include <stdlib.h>



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

void peers_free(PeerList* list) {
	free(list->peers);
	free(list);
}
