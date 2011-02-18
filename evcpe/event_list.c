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
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

static struct evcpe_event *evcpe_event_list_find(
		struct evcpe_event_list *list, const char *event_code);

void evcpe_event_list_init(struct evcpe_event_list *list)
{
	TAILQ_INIT(&list->head);
}

void evcpe_event_list_clear(struct evcpe_event_list *list)
{
	struct evcpe_event *event;

	evcpe_debug(__func__, "clearing evcpe_event_list");

	while((event = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, event, entry);
		free(event);
	}
	list->size = 0;
}

int evcpe_event_list_clone(struct evcpe_event_list *src,
		struct evcpe_event_list *dst)
{
	int rc;
	struct evcpe_event *src_ev, *dst_ev;

	evcpe_debug(__func__, "cloning evcpe_event_list");

	TAILQ_FOREACH(src_ev, &src->head, entry) {
		if ((rc = evcpe_event_list_add(dst, &dst_ev,
				src_ev->event_code, src_ev->command_key))) {
			evcpe_error(__func__, "failed to add event to destination list");
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

unsigned int evcpe_event_list_size(struct evcpe_event_list *list)
{
	return list->size;
}

struct evcpe_event *evcpe_event_list_find(
		struct evcpe_event_list *list, const char *event_code)
{
	struct evcpe_event *event;

	evcpe_trace(__func__, "finding event: %s", event_code);

	TAILQ_FOREACH(event, &list->head, entry) {
		if (!strcmp(event_code, event->event_code))
			return event;
	}
	return NULL;
}

int evcpe_event_list_add(struct evcpe_event_list *list,
		struct evcpe_event **event,
		const char *event_code, const char *command_key)
{
	int rc;
	struct evcpe_event *ev;

	if (!event_code || !command_key) return EINVAL;
	if (strlen(event_code) >= sizeof(ev->event_code) ||
			strlen(command_key) >= sizeof(ev->command_key))
		return EOVERFLOW;

	if (strncmp("M ", event_code, 2) &&
			(ev = evcpe_event_list_find(list, event_code))) {
		return 0;
	}

	evcpe_debug(__func__, "adding event: %s", event_code);

	if (!(ev = calloc(1, sizeof(struct evcpe_event)))) {
		evcpe_error(__func__, "failed to calloc evcpe_event");
		rc = ENOMEM;
		goto finally;
	}
	strcpy(ev->event_code, event_code);
	strcpy(ev->command_key, command_key);
	TAILQ_INSERT_TAIL(&list->head, ev, entry);
	*event = ev;
	list->size ++;
	rc = 0;

finally:
	return rc;
}

void evcpe_event_list_remove(struct evcpe_event_list *list,
		struct evcpe_event *event)
{
	TAILQ_REMOVE(&list->head, event, entry);
	list->size --;
	free(event);
}

void evcpe_event_list_remove_event(struct evcpe_event_list *list,
		const char *event_code)
{
	struct evcpe_event *event;

	TAILQ_FOREACH(event, &list->head, entry) {
		if (!strcmp(event_code, event->event_code)) {
			evcpe_event_list_remove(list, event);
			break;
		}
	}
}
