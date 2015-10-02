// $Id$
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EVCPE_TYPE_H_
#define EVCPE_TYPE_H_

#include <sys/types.h>

#include "constraint.h"

typedef enum _evcpe_type {
	EVCPE_TYPE_OBJECT,
	EVCPE_TYPE_MULTIPLE,
	EVCPE_TYPE_STRING,
	EVCPE_TYPE_INT,
	EVCPE_TYPE_UNSIGNED_INT,
	EVCPE_TYPE_UNSIGNED_LONG,
	EVCPE_TYPE_BOOLEAN,
	EVCPE_TYPE_DATETIME,
	EVCPE_TYPE_BASE64,

	/* TR-069 Data Model Data Types */

	EVCPE_TYPE_ALIAS,                     // string(64)
	EVCEP_TYPE_DBM_1000,                  // int
	EVCPE_TYPE_IEEE_EUI64,                // string(23)
	EVCPE_TYPE_IP_ADDRESS,                // string(45)
	EVCPE_TYPE_IP_PREFIX,                 // string(49)
	EVCPE_TYPE_IP_V4_ADDRESS,             // IPAddress(15)
	EVCPE_TYPE_IP_V4_PREFIX,              // IPPrefix(18)
	EVCPE_TYPE_IP_V6_ADDRESS,             // IPAddress
	EVCEP_TYPE_IP_V6_PREFIX,              // IPPrefix
	EVCPE_TYPE_MAC_ADDRESS,               // string(17)
	EVCPE_TYPE_STATS_COUNTER_32,          // unsignedInt
	EVCPE_TYPE_STATS_COUNTER_64,          // unsignedLong
	EVCPE_TYPE_UUID,                      // string(36:36)
	EVCPE_TYPE_ZIG_BEE_NETWORK_ADDRESS,   // string(4)
	EVCPE_TYPE_UNKNOWN
} evcpe_type_t;

const char *evcpe_type_to_str(evcpe_type_t type);

evcpe_type_t evcpe_type_from_str(const char *type_string, unsigned len);

int evcpe_type_validate(evcpe_type_t type, const char *value, unsigned len,
		evcpe_constraint *cons, const char *pattern);

#endif /* EVCPE_TYPE_H_ */
