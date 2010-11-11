#ifndef __SVC_COMMON_NET_H
#define __SVC_COMMON_NET_H

#include <sys/time.h>

/**
 * Holds acknowledgments sent.
 */
typedef struct _NetPacketAcknowledgments {
	struct _NetPacketAcknowledgments* next;
	unsigned int seq_num;
	struct timeval time;
} NetPacketAcknowledgments;


/**
 * Holds packets that got sent and wern't confirmed yet.
 */
typedef struct _NetPacketUnconfirmed {
	struct _NetPacketUnconfirmed* next;
	dstring* packet;
	unsigned int seq_num;
	struct timeval last_send;
	unsigned int retries;
} NetPacketUnconfirmed;


void net_packet_ack_add(NetPacketAcknowledgments** acknowledgements, NetPacketAcknowledgments* ack);
void net_packet_sent_add(NetPacketUnconfirmed** packets, NetPacketUnconfirmed* packet);


#endif /* !__SVC_COMMON_NET_H */
