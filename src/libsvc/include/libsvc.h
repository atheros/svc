typedef void* packet_t;
typedef void* peer_t;
typedef void svc_send_packet_t(packet_t);

void svc_init(svc_send_packet_t send_callback);
void svc_play(packet_t packet, peer_t peer);
