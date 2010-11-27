/*
 * channels.h
 *
 *  Created on: 18-11-2010
 *      Author: przemek
 */

#ifndef CHANNELS_H_
#define CHANNELS_H_



typedef struct {
	unsigned int	parent_channel;
	unsigned int	next_channel;

	/**
	 * if non 0, when automaticly moving clients up, this node will be ommited,
	 * since it is going to be removed.
	 */
	int				to_be_deleted;

	unsigned int	id;
	dstring			*name;

	unsigned int	peers_count;
	unsigned int	peers_size;
	unsigned int*	peers;
} Channel;


typedef struct {
	Channel*		channels;
	unsigned int	size;
	unsigned int	count;
} Channels;


Channels* channels_alloc(const char* rootName);
void channels_free(Channels* channels);

int channels_add(Channels* channels, int parent, const char* name);

/**
 * Remove a channel.
 *
 * All child channels will be removed.
 * All peers will be moved to the parent of channel_id
 *
 * NOTE: Root channel CAN NOT be removed!
 */
int channels_remove(Channels* channels, int id);

int channels_add_peer(Channels* channel, int channel_id, int peer_id);
int channels_remove_peer(Channels* channel, int channel_id, int peer_id);

#endif /* CHANNELS_H_ */
