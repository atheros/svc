/*
 * channels.c
 *
 *  Created on: 27-11-2010
 *      Author: przemek
 */
#include <stdlib.h>
#include "channels.h"

Channels* channels_alloc(const char* rootName) {
	Channels* channels = (Channels*)malloc(sizeof(Channels));
	channels->size = 1;
	channels->count = 1;
	channels->channels = (Channel*)malloc(sizeof(Channel));

	channels->channels[0].id = 0;
	channels->channels[0].used = 1;
	channels->channels[0].name = dfromcs(rootName);
	channels->channels[0].next_channel = CHANNEL_NULL;
	channels->channels[0].prev_channel = CHANNEL_NULL;
	channels->channels[0].parent_channel = CHANNEL_NULL;
	channels->channels[0].peers_count = 0;
	channels->channels[0].peers_size = 0;
	channels->channels[0].peers = NULL;

	return channels;
}

void channels_free(Channels* channels) {
	int ch;
	for(ch = 0; ch < channels->size; ch++) {
		if (!channels->channels[ch].used) continue;

		if (channels->channels[ch].peers_size) {
			free(channels->channels[ch].peers);
		}

		dfree(channels->channels[ch].name);
	}

	free(channels->channels);
	free(channels);
}

int channels_add(Channels* channels, int parent, const char* name);
int channels_remove(Channels* channels, int id);

int channels_add_peer(Channels* channel, int channel_id, int peer_id);
int channels_remove_peer(Channels* channel, int channel_id, int peer_id);
