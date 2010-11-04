#define __USE_GNU
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "libsvc.h"

extern int errno;

typedef struct _Peer {
	peer_t* peer;
	struct sockaddr_in address;
} Peer;

static Peer* peers;
static int peers_count;
static struct sockaddr_in host;
static int host_sock;

void send_callback(network_packet_t* packet){
	int i;
	unsigned char* data = (unsigned char*)malloc(packet->data_len+2);
	
	data[0] = packet->time & 0xFF;
	data[1] = (packet->time >> 8) & 0xFF;
	memcpy(data + 2, packet->data, packet->data_len);

	for(i = 0; i < peers_count; i++) {
		sendto(host_sock, data, packet->data_len+2, 0,
			(struct sockaddr*)&(peers[i].address),
			sizeof(struct sockaddr_in));
	}
	
	free(data);
}


int main(int argc, char* argv[]) {
	struct hostent *hp;
	fd_set rset;
	struct timeval tv;
	int i, port, r;
	unsigned char buffer[65535];
	struct sockaddr_in remote;
	network_packet_t np;
	socklen_t remote_len;
	
	if (argc < 3) {
		fprintf(stderr, "usage: %s port peer_address[, peer_address[...]]\n", argv[0]);
		return 1;
	}
	
	svc_init(send_callback);
	
	/* create local socket */
	host_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (host_sock < 0) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		return 1;
	}
	
	/* bind local socket */
	port = atoi(argv[1]);
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = htonl(INADDR_ANY);
	host.sin_port = htons(port);
	if (bind(host_sock, (struct sockaddr*)&host, sizeof(host))) {
		fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
		return 1;
	}
	
	
	/* init peers */
	peers_count = argc - 2;
	peers = (Peer*)malloc(sizeof(Peer) * peers_count);
	
	/* create each individual peer */
	for(i = 0; i < peers_count; i++) {
		peers[i].peer = svc_peer_join();
	}


	/* close peers */
	for(i = 0; i < peers_count; i++) {
		peers[i].address.sin_family = AF_INET;
		hp = gethostbyname(argv[i+3]);
		if (!hp) {
			fprintf(stderr, "Failed to resolve %s\n", argv[i+3]);
			return 1;
		}
		memcpy(&(peers[i].address.sin_addr.s_addr), hp->h_addr_list[0], hp->h_length);
		peers[i].address.sin_port = htons(port);
		svc_peer_leave(peers[i].peer);
	}
	
	while (1) {
		FD_ZERO(&rset);
		FD_SET(host_sock, &rset);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		r = select(host_sock+1, &rset, NULL, NULL, &tv);
		if (r == 0) {
			continue;
		} else if (r < 0) {
			fprintf(stderr, "select() failed: %s\n", strerror(errno));
			return 1;
		}
		
		remote_len = sizeof(remote);
		r = recvfrom(host_sock, buffer, 65536, 0,
				(struct sockaddr*)&remote, &remote_len);
		
		if (r <= 0) {
			continue;
		}
		
	
		for(i = 0; i < peers_count; i++) {
			if (peers[i].address.sin_port == remote.sin_port &&
				peers[i].address.sin_addr.s_addr == remote.sin_addr.s_addr) {
				
				np.data = (unsigned char*)(buffer+2);
				np.data_len = r - 2;
				np.time = ((unsigned char*)buffer)[0]
					| ((unsigned char*)buffer)[1] << 8;
				svc_packet_recieve(&np, peers[i].peer);
				break;
			}
		}
	}

	svc_close();
	return 0;
}
