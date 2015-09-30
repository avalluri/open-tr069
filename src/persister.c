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

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "repo.h"
#include "obj_xml.h"

#include "persister.h"

static int evcpe_persister_persist(struct evcpe_persister *persist);
static void evcpe_persister_timer_cb(int fd, short event, void *arg);
//static void evcpe_persister_write_cb(int fd, short event, void *arg);
static void evcpe_persister_listen_cb(struct evcpe_repo *repo,
		enum evcpe_attr_event event, const char *param_name, void *cbarg);

struct evcpe_persister *evcpe_persister_new(struct event_base *evbase)
{
	struct evcpe_persister *persist;

	DEBUG("constructing evcpe_persister");

	if (!(persist = calloc(1, sizeof(struct evcpe_persister)))) {
		ERROR("failed to calloc evcpe_persister");
		return NULL;
	}
	if (!(persist->buffer = evbuffer_new())) {
		ERROR("failed to create evbuffer");
		free(persist);
		return NULL;
	}
	persist->evbase = evbase;
//	persist->timer_tv.tv_sec = 5;
	return persist;
}

void evcpe_persister_free(struct evcpe_persister *persist)
{
	if (!persist) return;

	DEBUG("destructing evcpe_persister");

	if (event_initialized(&persist->timer_ev) && evtimer_pending(
			&persist->timer_ev, NULL)) {
		event_del(&persist->timer_ev);
	}
	if (evtimer_initialized(&persist->timer_ev) && evtimer_pending(
			&persist->timer_ev, NULL)) {
		event_del(&persist->timer_ev);
	    if (evcpe_persister_persist(persist))
		    ERROR("failed to write buffer");
	}
	if (persist->buffer)
	  evbuffer_free(persist->buffer);

	free(persist);
}

int evcpe_persister_set(struct evcpe_persister *persist,
		struct evcpe_repo *repo, const char *filename)
{
	int rc;

	DEBUG("setting persisting target: %s", filename);

	if ((rc = evcpe_repo_listen(repo, evcpe_persister_listen_cb, persist))) {
		ERROR("failed to listen repo");
		goto finally;
	}
	persist->repo = repo;
	persist->filename = filename;
	rc = 0;

finally:
	return rc;
}

void evcpe_persister_listen_cb(struct evcpe_repo *repo,
		enum evcpe_attr_event event, const char *param_name, void *cbarg)
{
	int rc;
	struct evcpe_persister *persist = cbarg;

	DEBUG("kicking persister");

	if (!event_initialized(&persist->timer_ev)) {
		evtimer_set(&persist->timer_ev, evcpe_persister_timer_cb, persist);
		if ((rc = event_base_set(persist->evbase, &persist->timer_ev))) {
			ERROR("failed to set event base");
			goto finally;
		}
	}
	if (!evtimer_pending(&persist->timer_ev, NULL)) {
		if ((rc = event_add(&persist->timer_ev, &persist->timer_tv))) {
			ERROR("failed to add timer event");
			goto finally;
		}
	}
	rc = 0;

finally:
	return;
}

int evcpe_persister_persist(struct evcpe_persister *persist)
{
	int rc, fd;
	FILE *fp;

	DEBUG("persisting repository");

	if (!(fp = fopen(persist->filename, "w+"))) {
		ERROR("failed to open file to write: %s", persist->filename);
		rc = errno ? errno : -1;
		goto finally;
	}
	fd = fileno(fp);
	evbuffer_drain(persist->buffer, evbuffer_get_length(persist->buffer));
	if ((rc = evcpe_obj_to_xml(persist->repo->root, persist->buffer))) {
		ERROR("failed to marshal root object");
		goto finally;
	}
	DEBUG("%.*s", (int)evbuffer_get_length(persist->buffer),
			evbuffer_pullup(persist->buffer ,-1));
    while (evbuffer_get_length(persist->buffer)) {
    	if (evbuffer_write(persist->buffer, fd) < 0) {
			ERROR("failed to write buffer");
			rc = errno ? errno : -1;
			goto finally;
    	}
    }
	rc = 0;

finally:
	if (fp) fclose(fp);
	return rc;
}

void evcpe_persister_timer_cb(int fd, short event, void *arg)
{
	struct evcpe_persister *persist = arg;

	DEBUG("starting timer callback");

	if (evcpe_persister_persist(persist))
		ERROR("failed to persist");
}

