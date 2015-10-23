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

#ifndef EVCPE_URL_H_
#define EVCPE_URL_H_

#include <sys/types.h>
#include <event.h>
#include <stdio.h>

typedef struct _evcpe_url {
	char *protocol;
	char *username;
	char *password;
	char *host;
	u_short port;
	char *uri;
} evcpe_url;

u_short evcpe_url_default_port(const char *protocol);

evcpe_url *evcpe_url_new(void);

evcpe_url* evcpe_url_new_from_str(const char* str);

void evcpe_url_free(evcpe_url *url);

void evcpe_url_reset(evcpe_url *url);

int evcpe_url_from_str(evcpe_url *url, const char *str);

#endif /* EVCPE_URL_H_ */
