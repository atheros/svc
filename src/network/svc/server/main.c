#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "peer.h"
#include "dstrutils.h"


static struct option options[] = {
		{"bind",		required_argument,	NULL, 'b'},
		{"help",		no_argument,		NULL, 'h'},
		{"max-peers",	required_argument,	NULL, 'm'},
		{"port",		required_argument,	NULL, 'p'},
		{NULL, 0, NULL, 0}
};


static unsigned int max_peers;
static unsigned int port;
static char bindto[100];

void help(const char* app) {
	printf("usage: %s [options]\n", app);
	printf("Options:\n");
	printf("  -p,--port=PORT                    Specify server port number (default 49001).\n");
	printf("  -m,--max-peers=MAX                Specify maximum peers count (default 16).\n");
	printf("  -b,--bind=ADDR                    Bind to specified address (default 0.0.0.0).\n");
}

int main(int argc, char* argv[]) {
	int i, ptr;

	/* default values */
	max_peers = 16;
	port = 49001;
	strcpy(bindto, "0.0.0.0");

	while ((i = getopt_long(argc, argv, "b:hm:p:", options, &ptr)) >= 0) {
		switch (i) {
		case 'b':
			 strncpy(bindto, optarg, 100);
			 bindto[99] = 0;
			 break;

		case 'h':
			help(argv[0]);
			return 0;

		case 'm':
			max_peers = atoi(optarg);
			if (max_peers < 2) {
				fprintf(stderr, "Max peers must be atleast 2\n");
				return 1;
			}
			break;

		case 'p':
			port = atoi(optarg);
			if (port == 0 || port > 65535) {
				fprintf(stderr, "Invalid port number %s\n", optarg);
				return 1;
			}
			break;

		}
	}

	return 0;
}
