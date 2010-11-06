#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>

#include "audio_api.h"

jack_client_t *client;
jack_port_t *input_port;
jack_port_t *output_port;

audio_callback_t interface_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

int process (jack_nframes_t nframes, void *arg)
{
	output_audio_data->data = (float *) jack_port_get_buffer (output_port, nframes);
	input_audio_data->data = (float *) jack_port_get_buffer (input_port, nframes);

	interface_callback(input_audio_data, output_audio_data);
	return 0;      
}

int strcnt (char list[], char c) {
	int i;
	int count = 0;

	for (i = 0; list[i] != '\0'; i++)
		if (list[i] == c)
			count++;

	return (count);
}

const char **getenv_array(const char *env) {
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

int init_audio (uint_fast16_t rate, uint_fast32_t frame_size)
{
	input_audio_data  = malloc(sizeof(audio_data_t));
	output_audio_data = malloc(sizeof(audio_data_t));
	
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;

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
	return 0;
}

int set_audio_callback(audio_callback_t audio_callback) {
	interface_callback = audio_callback;
	return 0;
}

