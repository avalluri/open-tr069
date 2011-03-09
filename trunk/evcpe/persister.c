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

	evcpe_debug(__func__, "constructing evcpe_persister");

	if (!(persist = calloc(1, sizeof(struct evcpe_persister)))) {
		evcpe_error(__func__, "failed to calloc evcpe_persister");
		return NULL;
	}
	if (!(persist->buffer = evbuffer_new())) {
		evcpe_error(__func__, "failed to create evbuffer");
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

	evcpe_debug(__func__, "destructing evcpe_persister");

	if (event_initialized(&persist->timer_ev) && evtimer_pending(
			&persist->timer_ev, NULL)) {
		event_del(&persist->timer_ev);
	}
	if (evtimer_initialized(&persist->timer_ev) && evtimer_pending(
			&persist->timer_ev, NULL)) {
		event_del(&persist->timer_ev);
	    if (evcpe_persister_persist(persist))
		    evcpe_error(__func__, "failed to write buffer");
	}
	if (persist->buffer)
	  evbuffer_free(persist->buffer);

	free(persist);
}

int evcpe_persister_set(struct evcpe_persister *persist,
		struct evcpe_repo *repo, const char *filename)
{
	int rc;

	evcpe_debug(__func__, "setting persisting target: %s", filename);

	if ((rc = evcpe_repo_listen(repo, evcpe_persister_listen_cb, persist))) {
		evcpe_error(__func__, "failed to listen repo");
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

	evcpe_debug(__func__, "kicking persister");

	if (!event_initialized(&persist->timer_ev)) {
		evtimer_set(&persist->timer_ev, evcpe_persister_timer_cb, persist);
		if ((rc = event_base_set(persist->evbase, &persist->timer_ev))) {
			evcpe_error(__func__, "failed to set event base");
			goto finally;
		}
	}
	if (!evtimer_pending(&persist->timer_ev, NULL)) {
		if ((rc = event_add(&persist->timer_ev, &persist->timer_tv))) {
			evcpe_error(__func__, "failed to add timer event");
			goto finally;
		}
	}
	rc = 0;

finally:
	return;
}

#if 0
int evcpe_persister_write(struct evcpe_persister *persist)
{
	int rc;

	evcpe_debug(__func__, "writing data in buffer");

	if (EVBUFFER_LENGTH(persist->buffer) > persist->written) {
	    if ((rc = write(persist->fd,
	    		EVBUFFER_DATA(persist->buffer) + persist->written,
	    		EVBUFFER_LENGTH(persist->buffer) - persist->written)) == -1) {
		    if (errno == EAGAIN || errno == EINTR || errno == EINPROGRESS)
			    goto reschedule;
		    else
				goto finally;
	    }
	    persist->written += rc;
	}

reschedule:
	if (EVBUFFER_LENGTH(persist->buffer) > persist->written) {
			if ((rc = event_add(&persist->write_ev, NULL))) {
				evcpe_error(__func__, "failed to add write event");
				goto finally;
			}
	} else {
		evbuffer_drain(persist->buffer, EVBUFFER_LENGTH(persist->buffer));
		close(persist->fd);
	}
	rc = 0;

finally:
	return rc;
}
#endif

int evcpe_persister_persist(struct evcpe_persister *persist)
{
	int rc, fd;
	FILE *fp;

	evcpe_debug(__func__, "persisting repository");

	if (!(fp = fopen(persist->filename, "w+"))) {
		evcpe_error(__func__, "failed to open file to write: %s", persist->filename);
		rc = errno ? errno : -1;
		goto finally;
	}
	fd = fileno(fp);
	evbuffer_drain(persist->buffer, EVBUFFER_LENGTH(persist->buffer));
	if ((rc = evcpe_obj_to_xml(persist->repo->root, persist->buffer))) {
		evcpe_error(__func__, "failed to marshal root object");
		goto finally;
	}
	evcpe_debug(__func__, "%.*s", EVBUFFER_LENGTH(persist->buffer),
			EVBUFFER_DATA(persist->buffer));
    while (EVBUFFER_LENGTH(persist->buffer)) {
    	if (evbuffer_write(persist->buffer, fd) < 0) {
			evcpe_error(__func__, "failed to write buffer");
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

	evcpe_debug(__func__, "starting timer callback");

	if (evcpe_persister_persist(persist))
		evcpe_error(__func__, "failed to persist");
}
#if 0
void evcpe_persister_write_cb(int fd, short event, void *arg)
{
	struct evcpe_persister *persist = arg;

	evcpe_debug(__func__, "starting write callback");

	if (evcpe_persister_write(persist)) {
		evcpe_error(__func__, "failed to write buffer");
		close(persist->fd);
		return;
	}
}
#endif
