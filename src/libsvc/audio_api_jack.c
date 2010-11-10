#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>

#include "audio_api.h"

static jack_client_t *client;
static jack_port_t *input_port;
static jack_port_t *output_port;

static capture_audio_callback_t jack_capture_callback;
static playback_audio_callback_t jack_playback_callback;

static audio_data_t* input_audio_data;
static audio_data_t* output_audio_data;

static int process (jack_nframes_t nframes, void *arg)
{
	output_audio_data->data = (float *) jack_port_get_buffer (output_port, nframes);
	input_audio_data->data = (float *) jack_port_get_buffer (input_port, nframes);

	jack_capture_callback(input_audio_data);
	jack_playback_callback(output_audio_data);
	return 0;      
}

static int strcnt (char list[], char c) {
	int i;
	int count = 0;

	for (i = 0; list[i] != '\0'; i++)
		if (list[i] == c)
			count++;

	return (count);
}

static const char **getenv_array(const char *env) {
	const char *raw = getenv(env);
	if (!raw)
		return NULL;

	char *value = (char *)strdup(raw);
	int size = strcnt(value, ' ') + 2;
	const char **tokens = malloc(sizeof (char*) * size);

	int i;
	tokens[0] = strtok(value, " ");
	for (i = 1; i < size; i++) {
		tokens[i] = strtok(NULL, " ");
	}
	tokens[i + 1] = NULL;

	return tokens;
}

int init_audio (unsigned int rate, unsigned int frame_size)
{	
	input_audio_data = audio_fake_data_create(frame_size);
	output_audio_data = audio_fake_data_create(frame_size);

	char client_name[10];
	sprintf(client_name, "SVC-%d", getpid());
	
	if ((client = jack_client_open (client_name, (jack_options_t) 0, NULL)) == 0) {
		fprintf (stderr, "jack server not running?\n");
		return -1;
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	   */

	jack_set_process_callback (client, process, 0);

	/* create two ports */

	input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	/* tell the JACK server that we are ready to roll */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return -1;
	}

	/* connect the ports. */

	const char **ports = getenv_array("SVC_JACK_INPUT");
	if (!ports) {
		if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
			fprintf(stderr, "Cannot find any physical capture ports\n");
			return -1;
		}
	}

	int i = 0;
	while (ports[i] != NULL) {
		if (jack_connect (client, ports[i], jack_port_name (input_port))) {
			fprintf (stderr, "cannot connect input ports\n");
		}
		i++;
	}

	ports = getenv_array("SVC_JACK_OUTPUT");
	if (!ports) {
		if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
			fprintf(stderr, "Cannot find any physical playback ports\n");
			return -1;
		}
	}

	i = 0;
	while (ports[i] != NULL) {
		if (jack_connect (client, jack_port_name (output_port), ports[i])) {
			fprintf (stderr, "cannot connect output ports\n");
		}
		i++;
	}
	
	return 0;
}

int close_audio () {
	jack_client_close (client);
	audio_fake_data_destroy(input_audio_data);
	audio_fake_data_destroy(output_audio_data);
	return 0;
}

int set_audio_callbacks(capture_audio_callback_t capture_callback, 
                        playback_audio_callback_t playback_callback){
	jack_capture_callback = capture_callback;
	jack_playback_callback = playback_callback;
	return 0;
}
