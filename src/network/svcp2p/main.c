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

#include <signal.h>

#include "libsvc.h"
#include "thread.h"

extern int errno;

/* global stats */
static mutex_t global_stats_lock;
static thread_t global_stats_thread;
static int up_bytes = 0;
static int down_bytes = 0;
static int up_packets = 0;
static int down_packets = 0;



typedef struct _Peer {
	svc_peer_t* peer;
	struct sockaddr_in address;
} Peer;

static Peer* peers;
static int peers_count;
static struct sockaddr_in host;
static int host_sock;
static int initialized = 0;
static int done = 0;

/**
 * Returns time in milliseconds between time in 2 timeval structures.
 */
static int msdiff(struct timeval *start, struct timeval *end) {
	long ms;
	ms = (end->tv_sec - start->tv_sec) * 1000;
	ms+= (end->tv_usec - start->tv_usec) / 1000;
	return ms;
}

/**
 * Print nice global stats.
 */
static void* global_stats_runner(void* dummy) {
	struct timeval tv1, tv2;
	float slept;

	int l_up_bytes;
	int l_down_bytes;
	int l_up_packets;
	int l_down_packets;
	
	float up_bps, down_bps;
	float up_pps, down_pps;
	
	while(1) {
		/* sleep 5 seconds */
		gettimeofday(&tv1, NULL);
		sleep(5);
		gettimeofday(&tv2, NULL);
		slept = ((float)msdiff(&tv1, &tv2))/1000.0;
		
		/* read and reset global stats */
		mutex_lock(&global_stats_lock);
		l_up_bytes = up_bytes;
		l_down_bytes = down_bytes;
		l_up_packets = up_packets;
		l_down_packets = down_packets;
		up_bytes = 0;
		down_bytes = 0;
		up_packets = 0;
		down_packets = 0;
		mutex_unlock(&global_stats_lock);
		
		up_bps = ((float)l_up_bytes) / slept / 1024;
		down_bps = ((float)l_down_bytes) / slept / 1024;
		up_pps = ((float)l_up_packets) / slept;
		down_pps = ((float)l_down_packets) / slept;
		
		printf("UP %.2fKBps (%.0fpps - %i), DOWN %.2fKBps (%.0fpps - %i)\n", up_bps, up_pps, l_up_packets, down_bps, down_pps, l_down_packets);
		
		if (done) {
			return NULL;
		}
	}
}

void send_callback(svc_network_packet_t* packet){
	int i;
	unsigned char* data;
	if (!initialized) return;
	
	
	data = (unsigned char*)malloc(packet->data_len+2);
	
	data[0] = packet->time & 0xFF;
	data[1] = (packet->time >> 8) & 0xFF;
	memcpy(data + 2, packet->data, packet->data_len);

	for(i = 0; i < peers_count; i++) {
		sendto(host_sock, data, packet->data_len+2, 0,
			(struct sockaddr*)&(peers[i].address),
			sizeof(struct sockaddr_in));
	}
	
	/* update stats */
	mutex_lock(&global_stats_lock);
	up_bytes+= (packet->data_len+2) * peers_count;
	up_packets+= peers_count;
	mutex_unlock(&global_stats_lock);
	
	
	free(data);
}

static void sigint() {
	printf("SIGINT, quitting.\n");
	done = 1;
}


int main(int argc, char* argv[]) {
	struct hostent *hp;
	fd_set rset;
	struct timeval tv;
	int i, port, r;
	unsigned char buffer[65535];
	struct sockaddr_in remote;
	svc_network_packet_t np;
	socklen_t remote_len;
	
	if (argc < 3) {
		fprintf(stderr, "usage: %s port peer_address[ peer_address[...]]\n", argv[0]);
		return 1;
	}
	
	
	
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
	memset(peers, 0, sizeof(Peer) * peers_count);
	
	/* resolve peers */
	for(i = 0; i < peers_count; i++) {
		printf("Resolving %s... ", argv[i+2]);
		peers[i].address.sin_family = AF_INET;
		hp = gethostbyname(argv[i+2]);
		if (!hp) {
			fprintf(stderr, "failed: %s\n", argv[i+2]);
			return 1;
		} else {
			printf("done\n");
		}
		memcpy(&(peers[i].address.sin_addr.s_addr), hp->h_addr_list[0], hp->h_length);
		peers[i].address.sin_port = htons(port);
	}

	
	/* create each individual peer */
	svc_init(send_callback);
	printf("Creating libsvc peers... ");
	for(i = 0; i < peers_count; i++) {
		peers[i].peer = svc_peer_join();
	}
	printf("done\n");
	
	/* create global stats thread */
	mutex_create(&global_stats_lock);
	thread_create(&global_stats_thread, global_stats_runner, NULL);
	

	printf("Host started.\n");
	initialized = 1;
	signal(SIGINT, sigint);
	while (!done) {
		FD_ZERO(&rset);
		FD_SET(host_sock, &rset);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		r = select(host_sock+1, &rset, NULL, NULL, &tv);
		if (r == 0) {
			continue;
		} else if (r < 0) {
			done = 1;
			continue;
		}
		
		remote_len = sizeof(remote);
		r = recvfrom(host_sock, buffer, 65536, 0,
				(struct sockaddr*)&remote, &remote_len);
		
		if (r <= 0) {
			continue;
		}
		
		/* update stats */
		mutex_lock(&global_stats_lock);
		down_bytes+= r;
		down_packets++;
		mutex_unlock(&global_stats_lock);
	
		/*printf("port %i\n", ntohs(remote.sin_port));*/
	
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
	signal(SIGINT, SIG_DFL);
	
	
	
	/* close peers */
	for(i = 0; i < peers_count; i++) {
		svc_peer_leave(peers[i].peer);
	}
	svc_close();

	printf("Waiting for threads... ");
	fflush(stdout);
	thread_join(global_stats_thread);
	mutex_destroy(&global_stats_lock);
	printf("done\n");
	
	
	return 0;
}
