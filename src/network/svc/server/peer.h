#ifndef __SVC_SERVER_PEER_H
#define __SVC_SERVER_PEER_H

#include "dstr.h"
#include "enet.h"

typedef struct {
	int			peer_id;
	dstring		*nick;
	dstring		*key;
	char		auth_key[32];
	peer_t		*peer;
	ENetPeer*	*enet;
} PeerInfo;


typedef struct {
	PeerInfo		*peers;
	unsigned int	size;
	unsigned int	count;
} PeerList;

PeerList* peers_alloc(unsigned int max);
void peers_free(PeerList* list);

#endif /* !__SVC_SERVER_PEER_H */
