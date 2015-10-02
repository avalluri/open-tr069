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

//#define _XOPEN_SOURCE // strptime()
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "util.h"

#include "type.h"

#ifdef __cplusplus__
extern "C" {
#endif

const char *types[] = {
  [EVCPE_TYPE_OBJECT] = "object",
  [EVCPE_TYPE_MULTIPLE] = "multipleObject",
  [EVCPE_TYPE_STRING] = "string",
  [EVCPE_TYPE_INT] = "int",
  [EVCPE_TYPE_UNSIGNED_INT] = "unsignedInt",
  [EVCPE_TYPE_UNSIGNED_LONG] = "unsignedLong",
  [EVCPE_TYPE_BOOLEAN] = "boolean",
  [EVCPE_TYPE_DATETIME] = "dateTime",
  [EVCPE_TYPE_BASE64] = "base64",

  [EVCPE_TYPE_ALIAS] = "Alias",
  [EVCEP_TYPE_DBM_1000] = "Dbm1000",
  [EVCPE_TYPE_IEEE_EUI64] = "IEEE_EUI64",
  [EVCPE_TYPE_IP_ADDRESS] = "IPAddress",
  [EVCPE_TYPE_IP_PREFIX] = "IPPrefix",
  [EVCPE_TYPE_IP_V4_ADDRESS] = "IPv4Address",
  [EVCPE_TYPE_IP_V4_PREFIX] = "IPv4Prefix",
  [EVCPE_TYPE_IP_V6_ADDRESS] = "IPv6Address",
  [EVCEP_TYPE_IP_V6_PREFIX] = "IPv6Prefix",
  [EVCPE_TYPE_MAC_ADDRESS] = "MACAddress",
  [EVCPE_TYPE_STATS_COUNTER_32] = "StatsCounter32",
  [EVCPE_TYPE_STATS_COUNTER_64] = "StatsCounter64",
  [EVCPE_TYPE_UUID] = "UUID",
  [EVCPE_TYPE_ZIG_BEE_NETWORK_ADDRESS] = "ZigBeeNetworkAddress"
};

#ifdef __cplusplus__
}
#endif

const char *evcpe_type_to_str(evcpe_type_t type)
{
  return types[type];
  /*
	switch (type) {
	case EVCPE_TYPE_OBJECT:
		return "object";
	case EVCPE_TYPE_MULTIPLE:
		return "multipleObject";
	case EVCPE_TYPE_STRING:
		return "string";
	case EVCPE_TYPE_INT:
		return "int";
	case EVCPE_TYPE_UNSIGNED_INT:
		return "unsignedInt";
	case EVCPE_TYPE_BOOLEAN:
		return "boolean";
	case EVCPE_TYPE_DATETIME:
		return "dateTime";
	case EVCPE_TYPE_BASE64:
		return "base64";
	case EVCPE_TYPE_UNKNOWN:
	default:
		return "unknown";
	}
  */
}

evcpe_type_t evcpe_type_from_str(const char *type_str, unsigned len)
{
  int i = 0;
  for(i = 0; i < EVCPE_TYPE_UNKNOWN; i++) {
		if (!evcpe_strncmp(types[i], type_str, len)) {
      return (evcpe_type_t)i;
    }
  }

  return EVCPE_TYPE_UNKNOWN;
}

int evcpe_type_validate(evcpe_type_t type, const char *value, unsigned len,
		evcpe_constraint *cons, const char *pattern)
{
	int rc = 0;
	long val = 0;
	char *dup = NULL;
	struct tm tm;

	DEBUG("validating value of type: %s", evcpe_type_to_str(type));

	if (type == EVCPE_TYPE_UNKNOWN || !cons) return 0;

	switch(type) {
	case EVCPE_TYPE_STRING:
		break;
	case EVCPE_TYPE_BASE64:
		// TODO
		break;
	case EVCPE_TYPE_BOOLEAN:
    if(!evcpe_strncmp("true", value, len) && 
       !evcpe_strncmp("false", value, len) &&
       !evcpe_strncmp("0", value, len) &&
       !evcpe_strncmp("1", value, len)) {
       ERROR("invalid boolean value: %.*s,"
        "valid boolean values are 'true, false, 0 or 1'", len, value);
      goto finally;
    }
    break;
	case EVCPE_TYPE_INT:
	case EVCPE_TYPE_UNSIGNED_INT:
		if ((rc = evcpe_atol(value, len, &val))) {
			ERROR("failed to convert to "
					"integer: %.*s", len, value);
			goto finally;
		}
		if (type == EVCPE_TYPE_UNSIGNED_INT) {
			if (val < 0) {
				ERROR("not a positive integer: %ld", val);
				goto finally;
			}
		}
		switch(cons->type) {
		case EVCPE_CONSTRAINT_NONE:
			break;
		case EVCPE_CONSTRAINT_MIN:
		case EVCPE_CONSTRAINT_MAX:
		case EVCPE_CONSTRAINT_RANGE:
			if (cons->type != EVCPE_CONSTRAINT_MAX &&
					val < cons->value.range.min) {
				ERROR("value out of range: %ld < %ld",
						val, cons->value.range.min);
				rc = EINVAL;
				goto finally;
			}
			if (cons->type != EVCPE_CONSTRAINT_MIN &&
					val > cons->value.range.max) {
				ERROR("value out of range: %ld > %ld",
						val, cons->value.range.max);
				rc = EINVAL;
				goto finally;
			}
			break;
		default:
			ERROR("unexpected constraint type: %d", cons->type);
			rc = EINVAL;
			goto finally;
		}
		break;
	case EVCPE_TYPE_DATETIME:
		if (!(dup = malloc(len + 1))) {
			ERROR("failed to malloc: %d bytes", len + 1);
			rc = ENOMEM;
			goto finally;
		}
		memcpy(dup, value, len);
		dup[len] = '\0';
		if (!strptime(dup, "%Y-%m-%dT%H:%M:%S", &tm)
				&& !strptime(dup, "%Y-%m-%dT%H:%M:%S%z", &tm)) {
			ERROR("failed to parse dateTime: %s", dup);
			free(dup);
			rc = EINVAL;
			goto finally;
		}
		free(dup);
		break;
	case EVCPE_TYPE_UNKNOWN:
	case EVCPE_TYPE_OBJECT:
	case EVCPE_TYPE_MULTIPLE:
	default:
		ERROR("value is not applicable to type: %d", type);
		rc = EINVAL; goto finally;
	}
	rc = 0;

finally:
	return rc;
}
