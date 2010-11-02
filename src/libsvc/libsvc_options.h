#ifndef __LIBSVC_OPTIONS_H_
#define __LIBSVC_OPTIONS_H_ 

typedef struct _svc_options_t {
	unsigned int sample_rate;
	unsigned int frame_size;
	unsigned int byte_per_packet;
} svc_options_t;

#endif /* __LIBSVC_OPTIONS_H_ */
