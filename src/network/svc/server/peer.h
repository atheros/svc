#ifndef __SVC_SERVER_PEER_H
#define __SVC_SERVER_PEER_H

#include <enet/enet.h>
#include "dstr.h"


typedef enet_uint32 PeerId;
#define PEER_NULL	0xffffffffU

typedef struct {
	int			used;
	PeerId		peer_id;
	dstring		*nick;
	dstring		*key;
	char		auth_key[32];
	int			initialized;
	ENetPeer*	*enet;
} PeerInfo;

typedef struct {
	PeerInfo		*peers;
	unsigned int	size;
	unsigned int	count;
} PeerList;



PeerList* peers_alloc(unsigned int max);
void peers_free(PeerList* peers);

PeerId peers_add(PeerList* peers);
void peers_remove(PeerList* peers, PeerId peer);

#endif /* !__SVC_SERVER_PEER_H */
