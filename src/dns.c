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

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <evdns.h>

#include "log.h"

#include "dns.h"

static void evcpe_dns_cb(int result, char type, int count, int ttl,
	    void *addresses, void *arg);

static void evcpe_dns_timer_cb(int fd, short event, void *arg);

static const char *evcpe_dns_resolve(const char *hostname);
