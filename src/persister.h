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

#ifndef EVCPE_PERSISTER_H_
#define EVCPE_PERSISTER_H_

#include <stdio.h>
#include <event.h>

struct evcpe_persister {
	struct event_base *evbase;
	struct evcpe_repo *repo;
	struct evbuffer *buffer;
//	struct event write_ev;
	struct event timer_ev;
	struct timeval timer_tv;
	const char *filename;
//	int fd;
//	size_t written;
};

struct evcpe_persister *evcpe_persister_new(struct event_base *evbase);

void evcpe_persister_free(struct evcpe_persister *persist);

int evcpe_persister_set(struct evcpe_persister *persist,
		struct evcpe_repo *repo, const char *filename);

#endif /* EVCPE_PERSISTER_H_ */
