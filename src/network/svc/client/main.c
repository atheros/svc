/*
 * main.c
 *
 *  Created on: 04-12-2010
 *      Author: przemek
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "dstrutils.h"

static unsigned int connectto_port;
static char connectto_host[100];
static int connectto = 0;

static struct option options[] = {
		{"connect",		required_argument,	NULL, 'c'},
		{"help",		no_argument,		NULL, 'h'},
		{NULL, 0, NULL, 0}
};

static void help(const char* app) {
	printf("usage: %s [options]\n", app);
	printf("Options:\n");
	printf("  -c,--connect=HOST[:PORT]          Connect to specified host[:port] (default port 49010).\n");
	printf("  -h,--help                         Show this help screen.\n");
}

int client_main() {
	int done = 0;
	/* init structures */
	/* main loop */
	while (!done) {
		/* handle server comm */
		/* handle CLI */

	}

	return 0;
}

int main(int argc, char* argv[]) {
	int i, ptr;
	dstrlist* list;

	/* default values */
	connectto_port = 49010;
	strcpy(connectto_host, "");

	while ((i = getopt_long(argc, argv, "c:h", options, &ptr)) >= 0) {
		switch (i) {
		case 'c':
			connectto = 1;

			list = dsplitcs_on_cs(optarg, ":", 2);


			strncpy(connectto_host, list->front->string->data, 100);
			connectto_host[99] = 0;
			if (list->size == 2) {
				connectto_port = atoi(list->back->string->data);
				if (connectto_port == 0 || connectto_port > 65535) {
					fprintf(stderr, "Invalid port number %s\n", list->back->string->data);
					return 1;
				}
			}
			break;

		case 'h':
			help(argv[0]);
			return 0;

		}
	}

	return client_main();
}
