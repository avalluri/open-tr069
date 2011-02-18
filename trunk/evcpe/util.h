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

#ifndef EVCPE_UTIL_H_
#define EVCPE_UTIL_H_

#include <event.h>

#include "evcpe-internal.h"

int evcpe_is_ipaddr(const char *address);

int evcpe_add_buffer(struct evbuffer *buffer, const char *fmt, ...) EVCPE_CHKFMT(2,3);

int evcpe_encode_base64(struct evbuffer *buffer, u_char *data, unsigned len);

int evcpe_strcmp(const char *a, unsigned alen, const char *b, unsigned blen);

inline int evcpe_strncmp(const char *a, const char *b, unsigned blen);

int evcpe_atol(const char *text, unsigned len, long *value);

char *evcpe_ltoa(long value);

int evcpe_strdup(const char *string, unsigned len, char **ptr);

#endif /* EVCPE_UTIL_H_ */
