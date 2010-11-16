#ifndef __SVC_SERVER_PEER_H
#define __SVC_SERVER_PEER_H

#include "dstr.h"
#include "net.h"

typedef struct {
	int			peer_id;
	dstring*	nick;
	dstring*	key;
	char		auth_key[32];
	
	/** list of all acknowledgments sent. */
	NetPacketAcknowledgments* acknowledgments;
	/** list of all reliable packets sent but not confirmed yet. */
	NetPacketUnconfirmed* packets;
	
} PeerInfo;


typedef struct {
	PeerInfo* peers;
	unsigned int size;
	unsigned int count;
} PeerList;

PeerList* peers_alloc(unsigned int max);
void peers_free(PeerList* list);

#endif /* !__SVC_SERVER_PEER_H */
