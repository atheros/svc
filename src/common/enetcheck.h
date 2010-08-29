/*
 * enetcheck.h
 *
 *  Created on: 2010-07-01
 *      Author: przemek
 */

#ifndef ENETCHECK_H_
#define ENETCHECK_H_

#include <enet/enet.h>

#if !defined(ENET_VERSION_MAJOR) || !defined(ENET_VERSION_MINOR) || !defined(ENET_VERSION_PATCH)
#error You have a very old ENet eversion, please update to version 1.3.x+
#endif

#if ENET_VERSION_MINOR < 3
#error You need atleast ENet version 1.3.0
#endif

#endif /* ENETCHECK_H_ */
