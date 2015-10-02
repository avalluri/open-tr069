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

#ifndef EVCPE_DNS_CACHE_H_
#define EVCPE_DNS_CACHE_H_

#include <sys/tree.h>
#include <time.h>
#include <event.h>

typedef struct _evcpe_dns_entry {
	char *name;
	char *address;
	struct event ev;
	struct timeval tv;
	struct evdns_base *dns_base;
	RB_ENTRY(_evcpe_dns_entry) entry;
} evcpe_dns_entry;

RB_HEAD(_evcpe_dns_cache, _evcpe_dns_entry);
typedef struct _evcpe_dns_cache evcpe_dns_cache;

int evcpe_dns_entry_cmp(evcpe_dns_entry *a, evcpe_dns_entry *b);

RB_PROTOTYPE(_evcpe_dns_cache, _evcpe_dns_entry, entry, evcpe_dns_entry_cmp);

int evcpe_dns_cache_add(evcpe_dns_cache *cache,
		const char *name, evcpe_dns_entry **entry);

evcpe_dns_entry *evcpe_dns_cache_find(
		evcpe_dns_cache *cache, const char *name);

const char *evcpe_dns_cache_get(evcpe_dns_cache *cache,
		const char *name);

inline void evcpe_dns_cache_remove(evcpe_dns_cache *cache,
		evcpe_dns_entry *entry);

void evcpe_dns_cache_clear(evcpe_dns_cache *cache);

#endif /* EVCPE_DNS_CACHE_H_ */
