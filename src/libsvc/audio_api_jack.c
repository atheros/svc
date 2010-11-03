#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>

#include "audio_api.h"

jack_client_t *client;
jack_port_t *input_port;
jack_port_t *output_port;

audio_callback_t pa_interface_callback;

audio_data_t* input_audio_data;
audio_data_t* output_audio_data;

int process (jack_nframes_t nframes, void *arg)
{
	output_audio_data->data = (float *) jack_port_get_buffer (output_port, nframes);
	input_audio_data->data = (float *) jack_port_get_buffer (input_port, nframes);

	pa_interface_callback(input_audio_data, output_audio_data);
	return 0;      
}

int init_audio (uint_fast16_t rate, uint_fast32_t frame_size)
{
	printf("Initializing jack client...\n");

	input_audio_data  = malloc(sizeof(audio_data_t));
	output_audio_data = malloc(sizeof(audio_data_t));
	
	input_audio_data->size = frame_size;
	output_audio_data->size = frame_size;

	char client_name[10];
	sprintf(client_name, "SVC-%d", getpid());
	
	if ((client = jack_client_new (client_name)) == 0) {
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

	/* connect the ports. Note: you can't do this before
	   the client is activated, because we can't allow
	   connections to be made to clients that aren't
	   running.
	   */

	const char **ports;

	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
		fprintf(stderr, "Cannot find any physical capture ports\n");
		return -1;
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port)),
		jack_connect (client, ports[1], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);

	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		fprintf(stderr, "Cannot find any physical playback ports\n");
		return -1;
	}

	if (jack_connect (client, jack_port_name (output_port), ports[0]),
	    jack_connect (client, jack_port_name (output_port), ports[1])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);

	printf(" done.\n Engine sample rate: %" PRIu32 "\n",
			jack_get_sample_rate (client));
	return 0;
}

int close_audio () {
	printf("Closing jack client...");
	jack_client_close (client);
	printf(" done.\n");
	return 0;
}

int set_audio_callback(audio_callback_t audio_callback) {
	pa_interface_callback = audio_callback;
	return 0;
}

