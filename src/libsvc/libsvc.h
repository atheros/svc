#ifndef __LIBSVC_H_
#define __LIBSVC_H_ 

typedef void* packet_t;
typedef int peer_id;
	
typedef void svc_send_callback_t(packet_t);

void svc_init(svc_send_callback_t send_callback);
void svc_packet_recieve(packet_t packet, peer_id peer);
void svc_close();

/* Call this function when you want to listen to one more peer*/
void svc_peer_join(peer_id peer);

/* Call this function when you don't have to listen to the peer anymore */
void svc_peer_leave(peer_id peer);

#endif
