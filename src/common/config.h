/*
 * config.h
 *
 *  Created on: 2010-06-27
 *      Author: przemek
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define SVCCD_PORT				49001

/* server listening port */
#define SVCSERVER_PORT 			49000
/* channels per connection */
#define SVCSERVER_MAX_CLIENTS		100
#define SVCSERVER_MAX_CHANNELS		2

/* byte order */
#define SVC_PLATFORM_LE
/*#define SVC_PLATFORM_BE*/


#endif /* CONFIG_H_ */
