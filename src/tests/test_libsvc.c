#include "libsvc.h"
#include "network_packet.h"
#include <stdio.h>

peer_t* peer_self;

void tst_callback(network_packet_t* packet){
	if(peer_self!=NULL){
		svc_packet_recieve(packet, peer_self);
	}
}

int main(){
	int xxx;
	peer_self = NULL;
	printf("initiating...\n");
	svc_init(tst_callback);
	printf("joining peer...\n");
	peer_self = svc_peer_join();
	printf("running...\n");
	printf("You should be able to here yourslef in the speakers\n");
	scanf("%d", &xxx);
	svc_close();
	return 0;
}
