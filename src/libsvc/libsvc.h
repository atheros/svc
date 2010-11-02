#ifndef __LIBSVC_H_
#define __LIBSVC_H_ 

#include "libsvc_incoming.h"
#include "libsvc_options.h"

/* The packet recieved by this function will be destroyed by libsvc after the callback call. */
typedef void(* svc_send_callback_t)(network_packet_t* packet);

void svc_init(svc_send_callback_t send_callback);
void svc_close();

#endif /* __LIBSVC_H_ */
