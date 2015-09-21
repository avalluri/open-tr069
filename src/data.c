// $Id$
/*
 * Copyright (C) 2015 Intel Corporation
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

const char *evcpe_event_code_type_to_str(enum evcpe_event_code_type type) {
  switch(type) {
    case EVCPE_EVENT_0_BOOTSTRAP: return "0 BOOTSTRAP";
    case EVCPE_EVENT_1_BOOT: return "1 BOOT";
    case EVCPE_EVENT_2_PERIODIC: return "2 PERIODIC";
    case EVCPE_EVENT_3_SCHEDULED: return "3 SCHEDULED";
    case EVCPE_EVENT_4_VALUE_CHANGE: return "4 VALUE CHANGE";
    case EVCPE_EVENT_5_KICKED: return "5 KICKED";
    case EVCPE_EVENT_6_CONNECTION_REQUEST: return "6 CONNECTION REQUEST";
    case EVCPE_EVENT_7_TRANSFER_COMPLETE: return "7 TRANSFER COMPLETE";
    case EVCPE_EVENT_8_DIAGNOSTICS_COMPLETE: return "8 DIAGNOSTICS COMPLETE";
    case EVCPE_EVENT_9_REQUEST_DOWNLOAD: return "9 REQUEST DOWNLOAD";
    case EVCPE_EVENT_10_AUTONOMOUS_TRANSFER_COMPLETE: return "10 AUTONOMOUS TRANSFER COMPLETE";
    case EVCPE_EVENT_11_DU_STATE_CHANGE_COMPLETE: return "11 DU STATE CHANGE COMPLETE";
    case EVCPE_EVENT_12_AUTONOMOUS_DU_STATE_CHANGE_COMPLETE: return "12 AUTONOMOUS DU STATE CHANGE COMPLETE";
    case EVCPE_EVENT_13_WAKEUP: return "13 WAKEUP";
    default: return NULL;
  }
}

struct evcpe_event *evcpe_event_new()
{
	return calloc(1, sizeof(struct evcpe_event));
}

int evcpe_event_clone(struct evcpe_event *src, struct evcpe_event **dst) {

	if (!dst || !src) return EINVAL;
	if ((*dst = evcpe_event_new()) == NULL) return ENOMEM;

	strcpy((*dst)->command_key, src->command_key);
	strcpy((*dst)->event_code, src->event_code);

	return 0;
}

struct evcpe_event *evcpe_event_list_find(
		struct evcpe_event_list *list, const char *event_code)
{
	struct evcpe_event *event;

	TRACE("finding event: %s", event_code);

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

	DEBUG("adding event: %s", event_code);

	if (!(ev = evcpe_event_new())) {
		ERROR("failed to calloc evcpe_event");
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

int evcpe_param_info_list_add(struct evcpe_param_info_list *list,
		struct evcpe_param_info **param, const char *name, unsigned len,
		int writable)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*param)->name)) return EOVERFLOW;

	DEBUG("adding param name: %.*s", len, name);

	if (!((*param) = calloc(1, sizeof(struct evcpe_param_info))))
		return ENOMEM;

	strncpy((*param)->name, name, len);
	(*param)->writable = writable;
	TAILQ_INSERT_TAIL(&list->head, (*param), entry);
	list->size ++;
	return 0;
}

int evcpe_param_name_list_add(struct evcpe_param_name_list *list,
		struct evcpe_param_name **param, const char *name, unsigned len)
{
	struct evcpe_param_name *param_name;

	if (!name || !len) return EINVAL;
	if (len >= sizeof(param_name->name)) return EOVERFLOW;

	DEBUG("adding param name: %.*s", len, name);

	if (!(param_name = calloc(1, sizeof(struct evcpe_param_name))))
		return ENOMEM;

	strncpy(param_name->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, param_name, entry);
	list->size ++;
	*param = param_name;
	return 0;
}

int evcpe_param_value_set(struct evcpe_param_value *param,
		const char *data, unsigned len)
{
	if (!param) return EINVAL;

	DEBUG("setting value: %s => %.*s",
			param->name, len, data);
	param->data = data;
	param->len = len;
	return 0;
}

int evcpe_param_value_list_add(struct evcpe_param_value_list *list,
		struct evcpe_param_value **value, const char *name, unsigned len)
{
	int rc;
	struct evcpe_param_value *param;

	if (!name || !len) return EINVAL;
	if (len >= sizeof(param->name)) return EOVERFLOW;

	DEBUG("adding parameter: %.*s", len, name);

	if (!(param = calloc(1, sizeof(struct evcpe_param_value)))) {
		ERROR("failed to calloc evcpe_param_value");
		rc = ENOMEM;
		goto finally;
	}
	strncpy(param->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, param, entry);
	*value = param;
	list->size ++;
	rc = 0;

finally:
	return rc;
}

struct evcpe_access* evcpe_access_new() {
	return (struct evcpe_access *)
			calloc(1, sizeof(struct evcpe_access));
}

int evcpe_access_clone(struct evcpe_access *src,
		struct evcpe_access **dst)
{
	if (!src || !dst) return EINVAL;
	if ((*dst = evcpe_access_new()) == NULL) return ENOMEM;
	strcpy((*dst)->entity, src->entity);

	return 0;
}

int evcpe_access_list_add(struct evcpe_access_list *list,
		const char *entity, unsigned len)
{
	struct evcpe_access *item = NULL;

	if (len >= sizeof(item->entity)) return EOVERFLOW;

	TRACE("adding entity: %.*s", len, entity);

	if (!(item = evcpe_access_new())) {
		return ENOMEM;
	}
	strncpy(item->entity, entity, len);
	TAILQ_INSERT_TAIL(&list->head, item, entry);
	list->size ++;
	return 0;
}


int evcpe_method_list_add(struct evcpe_method_list *list,
		struct evcpe_method **item, const char *name, unsigned len)
{
	if (len >= sizeof((*item)->name)) return EOVERFLOW;

	if (!((*item) = calloc(1, sizeof(struct evcpe_method)))) {
		ERROR("failed to calloc evcpe_method_list_item");
		return ENOMEM;
	}
	strncpy((*item)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *item, entry);
	list->size ++;
	return 0;
}

int evcpe_method_list_add_method(struct evcpe_method_list *list,
		const char *method)
{
	struct evcpe_method *item = NULL;
	return evcpe_method_list_add(list, &item, method, strlen(method));
}

int evcpe_set_param_attr_list_add(struct evcpe_set_param_attr_list *list,
		struct evcpe_set_param_attr **attr, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*attr)->name)) return EOVERFLOW;

	if (!(*attr = calloc(1, sizeof(struct evcpe_set_param_attr))))
		return ENOMEM;
	strncpy((*attr)->name, name, len);
	evcpe_access_list_init(&(*attr)->access_list);
	TAILQ_INSERT_TAIL(&list->head, *attr, entry);
	list->size ++;
	return 0;
}

int evcpe_set_param_value_list_add(struct evcpe_set_param_value_list *list,
		struct evcpe_set_param_value **value, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*value)->name)) return EOVERFLOW;

	if (!(*value = calloc(1, sizeof(struct evcpe_set_param_value))))
		return ENOMEM;
	strncpy((*value)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *value, entry);
	list->size ++;
	return 0;
}

int evcpe_set_param_value_set(struct evcpe_set_param_value *param,
		const char *data, unsigned len)
{
	if (!param || !data) return EINVAL;

	DEBUG("setting value: %s => %.*s", param->name, len, data);
	param->data = data;
	param->len = len;
	return 0;
}

int evcpe_param_attr_list_add(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr **attr, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*attr)->name)) return EOVERFLOW;

	DEBUG("adding param attr: %.*s", len, name);

	if (!(*attr = calloc(1, sizeof(struct evcpe_param_attr))))
		return ENOMEM;
	evcpe_access_list_init(&(*attr)->access_list);
	strncpy((*attr)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *attr, entry);
	list->size ++;
	return 0;
}

