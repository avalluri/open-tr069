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
#include <stdio.h>
#include <errno.h>

#include "log.h"
#include "fault.h"

#include "repo.h"

static int evcpe_repo_get_attr(struct evcpe_repo *repo, const char *name,
		struct evcpe_attr **attr);

static int evcpe_repo_locate(struct evcpe_repo *repo, const char *name,
		struct evcpe_obj **obj, struct evcpe_attr **attr, unsigned int *index);

static void evcpe_repo_set_obj_attr_cb(struct evcpe_repo *repo,
		struct evcpe_obj *obj);

static void evcpe_repo_attr_cb(struct evcpe_attr *attr,
		enum evcpe_attr_event event, void *data, void *cbarg);

struct evcpe_repo *evcpe_repo_new(struct evcpe_obj *root)
{
	struct evcpe_repo *repo;

	DEBUG("constructing evcpe_repo");

	if (!(repo = calloc(1, sizeof(struct evcpe_repo)))) {
		ERROR("failed to calloc evcpe_repo");
		return NULL;
	}
	TAILQ_INIT(&repo->listeners);
	evcpe_repo_set_obj_attr_cb(repo, root);
	repo->root = root;
	return repo;
}

void evcpe_repo_free(struct evcpe_repo *repo)
{
	struct evcpe_repo_listener *listener;

	if (!repo) return;

	DEBUG("destructing evcpe_repo");

	while((listener = TAILQ_FIRST(&repo->listeners))) {
		TAILQ_REMOVE(&repo->listeners, listener, entry);
		free(listener);
	}
	free(repo);
}

int evcpe_repo_listen(struct evcpe_repo *repo,
		evcpe_repo_listen_cb cb, void *arg)
{
	int rc;
	struct evcpe_repo_listener *listener;

	DEBUG("listening repository");

	if (!(listener = calloc(1, sizeof(struct evcpe_repo_listener)))) {
		ERROR("failed to calloc evcpe_repo_listener");
		rc = ENOMEM;
		goto finally;
	}
	listener->cb = cb;
	listener->cbarg = arg;
	TAILQ_INSERT_TAIL(&repo->listeners, listener, entry);
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_unlisten(struct evcpe_repo *repo,
		evcpe_repo_listen_cb cb)
{
	int rc;
	struct evcpe_repo_listener *listener, *match;

	DEBUG("unlistening repository");

	match = NULL;
	TAILQ_FOREACH(listener, &repo->listeners, entry) {
		if (cb == listener->cb) {
			match = listener;
			break;
		}
	}
	if (!match) {
		ERROR("listener not found");
		rc = EINVAL;
		goto finally;
	}
	TAILQ_REMOVE(&repo->listeners, match, entry);
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_locate(struct evcpe_repo *repo, const char *name,
		struct evcpe_obj **obj, struct evcpe_attr **attr, unsigned int *index)
{
	int rc = 0;
	const char *start, *end;

	*obj = repo->root;
	*attr = NULL;
	start = end = name;
	while(*end != '\0') {
		if (*end != '.') { end ++; continue; }

		if (end == start) {
			// expression neglecting root object (compatible to all
			// TR-069 enabled device
			*attr = RB_ROOT(&repo->root->attrs);
			*obj = (*attr)->value.object;
		} else if ((*attr) && (*attr)->schema->type == EVCPE_TYPE_MULTIPLE) {
			if (!(*index = atoi(start)) && errno) {
				ERROR("failed to convert to integer: %.*s",
						(int)(end - start), start);
				rc = ENOMEM;
				goto finally;
			}
			if (*index <= 0) {
				ERROR("invalid instance number: %d", *index);
				rc = ENOMEM;
				goto finally;
			}
			if ((rc = evcpe_attr_idx_obj(*attr, (*index) - 1, obj))) {
				ERROR("indexed object doesn't exist: [%d]", (*index) - 1);
				goto finally;
			}
		} else if ((rc = evcpe_obj_get(*obj, start, end - start, attr))) {
			ERROR("failed to get attribute: %.*s", (int)(end - start), start);
			goto finally;
		} else if ((*attr)->schema->type == EVCPE_TYPE_MULTIPLE) {
		} else if ((*attr)->schema->type == EVCPE_TYPE_OBJECT) {
			*obj = (*attr)->value.object;
		} else {
			ERROR("not an object/multiple attribute: %s", (*attr)->schema->name);
			rc = EVCPE_CPE_INVALID_PARAM_NAME;
			goto finally;
		}
		start = ++ end;
	}
	if (start != end) {
		if ((rc = evcpe_obj_get(*obj, start, end - start, attr))) {
			ERROR("failed to get attribute: %.*s", (int)(end - start), start);
			goto finally;
		} else if ((*attr)->schema->type == EVCPE_TYPE_OBJECT ||
				(*attr)->schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("not a simple attribute: %.*s", (int)(end - start), start);
			rc = EVCPE_CPE_INVALID_PARAM_NAME;
			goto finally;
		}
	}

finally:
	return rc;
}

int evcpe_repo_get_obj(struct evcpe_repo *repo, const char *name,
		struct evcpe_obj **ptr)
{
	int rc = 0;
	struct evcpe_obj *obj = NULL;
	struct evcpe_attr *attr = NULL;
	unsigned int index = 0;

	INFO("getting object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr) {
		ERROR("object not found: %s", name);
		rc = EINVAL;
		goto finally;
	} else if (attr->schema->type == EVCPE_TYPE_OBJECT) {
		*ptr = attr->value.object;
	} else if (attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		if (obj->class != attr->schema->class) {
			ERROR("missing instance number: %s", name);
			rc = EINVAL;
			goto finally;
		}
		*ptr = obj;
	} else {
		ERROR("not an object/multiple attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}

finally:
	return rc;
}

int evcpe_repo_get(struct evcpe_repo *repo, const char *name,
		const char **value, unsigned int *len)
{
	int rc = 0;
	struct evcpe_obj *obj = NULL;
	struct evcpe_attr *attr = NULL;
	unsigned int index = 0;

	DEBUG("getting parameter: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
	}
	else if ((rc = evcpe_attr_get(attr, value, len))) {
		ERROR("failed to get value: %s", name);
		rc = EINVAL;
	}

	return rc;
}

int evcpe_repo_set(struct evcpe_repo *repo, const char *name,
		const char *value, unsigned int len)
{
	int rc = 0;
	struct evcpe_obj *obj = NULL;
	struct evcpe_attr *attr = NULL;
	unsigned int index = 0;

	DEBUG("setting parameter: %s => %.*s", name, len, value);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if ((rc = evcpe_attr_set(attr, value, len))) {
		ERROR("failed to set value: %s", name);
		rc = EINVAL;
	}

finally:
	return rc;
}

int evcpe_repo_getcpy(struct evcpe_repo *repo, const char *name,
		char *value, unsigned int len)
{
	int rc = 0;
	const char *ptr;
	unsigned int ptrlen;

	if ((rc = evcpe_repo_get(repo, name, &ptr, &ptrlen)))
		goto finally;
	if (ptrlen >= len) {
		ERROR("value exceeds string limit: %d >= %d", ptrlen, len);
		rc = EOVERFLOW;
		goto finally;
	}
	memcpy(value, ptr, ptrlen);
	value[ptrlen] = '\0';

finally:
	return rc;
}

const char *evcpe_repo_find(struct evcpe_repo *repo, const char *name)
{
	struct evcpe_obj *obj;
	struct evcpe_attr *attr;
	unsigned int index, len;
	const char *value;

	DEBUG("finding parameter: %s", name);

	if (evcpe_repo_locate(repo, name, &obj, &attr, &index))
		return NULL;
	if (evcpe_attr_get(attr, &value, &len))
		return NULL;
	return value;
}

int evcpe_repo_get_attr(struct evcpe_repo *repo, const char *name,
		struct evcpe_attr **attr)
{
	int rc;
	struct evcpe_obj *obj;
	unsigned int index;

	DEBUG("getting attribute: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, attr, &index))) {
		ERROR("failed to locate: %s", name);
		goto finally;
	}
	if (!(*attr)) {
		ERROR("attribute not found: %s", name);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_add_obj(struct evcpe_repo *repo, const char *name,
		unsigned int *index)
{
	int rc;
	struct evcpe_obj *obj;
	struct evcpe_attr *attr;

	DEBUG("adding object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr || attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("not a multiple object attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_attr_add_obj(attr, &obj, index))) {
		ERROR("failed to add object: %s", name);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_del_obj(struct evcpe_repo *repo, const char *name)
{
	int rc;
	struct evcpe_obj *obj;
	struct evcpe_attr *attr;
	unsigned int index;

	DEBUG("deleting object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr || attr->schema->type != EVCPE_TYPE_OBJECT) {
		ERROR("not an object attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_attr_del_obj(attr, index))) {
		ERROR("failed to delete object: %s", name);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_get_objs(struct evcpe_repo *repo, const char *name,
		struct tqueue **list, unsigned int *size)
{
	int rc;
	struct evcpe_obj *obj;
	struct evcpe_attr *attr;
	unsigned int index;

	DEBUG("getting multiple objects: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("not multipleObject type: %s", name);
		rc = EVCPE_CPE_INVALID_PARAM_NAME;
		goto finally;
	}
	*list = attr->value.multiple.list;
	*size = attr->value.multiple.size;
	rc = 0;

finally:
	return rc;
}

void evcpe_repo_attr_cb(struct evcpe_attr *attr, enum evcpe_attr_event event,
		void *data, void *cbarg)
{
	struct evcpe_repo_listener *listener;
	struct evcpe_repo *repo = cbarg;

	if (event == EVCPE_ATTR_EVENT_OBJ_ADDED) {
		evcpe_repo_set_obj_attr_cb(repo, data);
	}
	TAILQ_FOREACH(listener, &repo->listeners, entry) {
		switch (event) {
		case EVCPE_ATTR_EVENT_OBJ_ADDED:
		case EVCPE_ATTR_EVENT_OBJ_DELETED:
			(*listener->cb)(repo, event,
					((struct evcpe_obj *)data)->path, listener->cbarg);
			break;
		case EVCPE_ATTR_EVENT_PARAM_SET:
			(*listener->cb)(repo, event, attr->path, listener->cbarg);
			break;
		}
	}
}

void evcpe_repo_set_obj_attr_cb(struct evcpe_repo *repo,
		struct evcpe_obj *obj)
{
	struct tqueue_element *item = NULL;
	struct evcpe_attr *attr = NULL;

	DEBUG("setting callback on attributes of %s", obj->path);

	RB_FOREACH(attr, evcpe_attrs, &obj->attrs) {
		evcpe_attr_set_cb(attr, evcpe_repo_attr_cb, repo);
		switch (attr->schema->type) {
		case EVCPE_TYPE_OBJECT:
			evcpe_repo_set_obj_attr_cb(repo, attr->value.object);
			break;
		case EVCPE_TYPE_MULTIPLE:
			TQUEUE_FOREACH(item, attr->value.multiple.list) {
				if (!item->data) continue;
				evcpe_repo_set_obj_attr_cb(repo, (struct evcpe_obj*)item->data);
			}
			break;
		default:
			break;
		}
	}
}

int evcpe_repo_add_event(struct evcpe_repo *repo,
		const char *event_code, const char *command_key)
{
	int rc;
	unsigned int index;
	const char *name;
	struct evcpe_obj *obj;
	struct evcpe_attr *attr, *param;

	DEBUG("adding event: %s - %s", event_code, command_key);

	name = ".Event.";
	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr || attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("not a multiple object attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_attr_add_obj(attr, &obj, &index))) {
		ERROR("failed to add object: %s", name);
		goto finally;
	}
	name = "EventCode";
	if ((rc = evcpe_obj_get(obj, name, strlen(name), &param))) {
		ERROR("failed to get parameter: %s", name);
		evcpe_attr_del_obj(attr, obj->index);
		goto finally;
	}
	if ((rc = evcpe_attr_set(param, event_code, strlen(event_code)))) {
		ERROR("failed to set parameter value: %s => %s",
				name, event_code);
		evcpe_attr_del_obj(attr, obj->index);
		goto finally;
	}
	name = "CommandKey";
	if ((rc = evcpe_obj_get(obj, name, strlen(name), &param))) {
		ERROR("failed to get parameter: %s", name);
		evcpe_attr_del_obj(attr, obj->index);
		goto finally;
	}
	if ((rc = evcpe_attr_set(param, command_key, strlen(command_key)))) {
		ERROR("failed to set parameter value: %s => %s",
				name, command_key);
		evcpe_attr_del_obj(attr, obj->index);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

static int evcpe_repo_find_event(struct tqueue *list,
		const char *event_code, struct evcpe_obj **obj)
{
	int rc = 0;
	struct tqueue_element* item = NULL;
	struct evcpe_attr *param;
	const char *code, *name = "EventCode";
	unsigned int len = 0;

	DEBUG("finding event: %s", event_code);

	*obj = NULL;
	TQUEUE_FOREACH(item, list) {
		struct evcpe_obj* tmp_obj = NULL;
		if (!(tmp_obj = item->data)) continue;
		if ((rc = evcpe_obj_get(tmp_obj, name, strlen(name), &param))) {
			ERROR("failed to get parameter: %s", name);
			goto finally;
		}
		if ((rc = evcpe_attr_get(param, &code, &len))) {
			ERROR("failed to get parameter value: %s", name);
			goto finally;
		}
		if (!strcmp(event_code, code)) {
			*obj = tmp_obj;
			break;
		}
	}

finally:
	return rc;
}

int evcpe_repo_del_event(struct evcpe_repo *repo,
		const char *event_code)
{
	int rc;
	unsigned int index;
	const char *name;
	struct evcpe_obj *obj, *child;
	struct evcpe_attr *attr;

	DEBUG("deleting event: %s", event_code);

	name = ".Event.";
	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr, &index))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr || attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("not a multiple object attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}
	while (!(rc = evcpe_repo_find_event(attr->value.multiple.list,
			event_code, &child)) && child) {
		if ((rc = evcpe_attr_del_obj(attr, child->index))) {
			ERROR("failed to delete object: %s", child->path);
			goto finally;
		}
	}
	if (rc) {
		ERROR("failed to find event: %s", event_code);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

static int evcpe_repo_to_inform_param_value_list(struct evcpe_obj *obj,
		struct evcpe_param_value_list *list)
{
	int rc = 0;
	struct evcpe_attr*     attr = NULL;
	struct tqueue_element* elm = NULL;

	TQUEUE_FOREACH(elm, obj->class->inform_attrs) {
		struct evcpe_attr_schema *schema = (struct evcpe_attr_schema *)elm->data;
		if (!(attr = evcpe_obj_find_deep(obj, schema))) {
			WARN("missing inform attribute '%s' in object", schema->name);
			continue;
		}

		/* sanity check */
		if (!schema->inform) break;
		if ((rc = evcpe_attr_to_param_value_list(attr, list))) {
			ERROR("failed to add param to value list");
			goto finally;
		}
	}

finally:
	return rc;
}

int evcpe_repo_to_inform(struct evcpe_repo *repo, struct evcpe_inform *inform)
{
	int rc = 0;
	struct tqueue *objs = NULL;
	struct tqueue_element *item = NULL;
	unsigned int count;
	struct evcpe_attr *attr;
	const char *code, *command;
	unsigned int len;
	struct evcpe_event *event;

	DEBUG("filling inform request");

	if ((rc = evcpe_repo_getcpy(repo, ".DeviceInfo.Manufacturer",
			inform->device_id.manufacturer,
			sizeof(inform->device_id.manufacturer)))) {
		ERROR("failed to get manufacturer");
		goto finally;
	}
	if ((rc = evcpe_repo_getcpy(repo, ".DeviceInfo.ManufacturerOUI",
			inform->device_id.oui,
			sizeof(inform->device_id.oui)))) {
		ERROR("failed to get manufacturer OUI");
		goto finally;
	}
	if ((rc = evcpe_repo_getcpy(repo, ".DeviceInfo.ProductClass",
			inform->device_id.product_class,
			sizeof(inform->device_id.product_class)))) {
		ERROR("failed to get product class");
		goto finally;
	}
	if ((rc = evcpe_repo_getcpy(repo, ".DeviceInfo.SerialNumber",
			inform->device_id.serial_number,
			sizeof(inform->device_id.serial_number)))) {
		ERROR("failed to get serial number");
		goto finally;
	}
	if ((rc = evcpe_repo_getcpy(repo, ".Time.CurrentLocalTime",
			inform->current_time, sizeof(inform->current_time)))) {
		ERROR("failed to get current local time");
		goto finally;
	}
	if ((rc = evcpe_repo_get_objs(repo, ".Event.", &objs, &count))) {
		ERROR("failed to get events");
		goto finally;
	}
	TQUEUE_FOREACH(item, objs) {
		struct evcpe_obj* obj = (struct evcpe_obj*)item->data;
		if (!obj) continue;
		if ((rc = evcpe_obj_get(obj, "EventCode", strlen("EventCode"),
				&attr)) || (rc = evcpe_attr_get(attr, &code, &len))) {
			ERROR("failed to get event code");
			goto finally;
		}
		if ((rc = evcpe_obj_get(obj, "CommandKey", strlen("CommandKey"),
				&attr)) || (rc = evcpe_attr_get(attr, &command, &len))) {
			ERROR("failed to get command key");
			goto finally;
		}
		if ((rc = evcpe_event_list_add(&inform->event, &event, code ? code : "",
				command ? command : ""))) {
			ERROR("failed to add event: %s", code);
			goto finally;
		}
	}
	if ((rc = evcpe_repo_to_inform_param_value_list(repo->root,
			&inform->parameter_list))) {
		ERROR("failed to add inform param value list");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_to_param_value_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_value_list *list)
{
	int rc;
	struct evcpe_attr *attr;

	DEBUG("adding attribute to parameter value list: %s", name);

	if ((rc = evcpe_repo_get_attr(repo, name, &attr))) {
		ERROR("failed to get attribute: %s", name);
		goto finally;
	}
	if ((rc = evcpe_attr_to_param_value_list(attr, list))) {
		ERROR("failed to add attribute to param list: %s", name);
		goto finally;
	}

finally:
	return rc;
}

int evcpe_repo_to_param_info_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_info_list *list, int next_level)
{
	int rc;
	struct evcpe_param_info *param;
	struct evcpe_attr *attr;

	DEBUG("adding attribute to parameter info list: %s", name);

	if ((rc = evcpe_repo_get_attr(repo, *name == '\0' ? "." : name, &attr))) {
		ERROR("failed to get attribute: %s", name);
		goto finally;
	}
	if (next_level && attr->schema->type != EVCPE_TYPE_OBJECT && attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		rc = EVCPE_CPE_INVALID_ARGUMENTS;
		goto finally;
	}
	if (*name == '\0') {
		if ((rc = evcpe_param_info_list_add(list, &param,
				attr->value.object->path, strlen(attr->value.object->path), attr->schema->write == 'W' ? 1 : 0))) {
			ERROR("failed to add param info");
			goto finally;
		}
	} else if ((rc = evcpe_attr_to_param_info_list(attr, list, next_level))) {
		ERROR("failed to add attribute to param info list: %s", name);
		goto finally;
	}

finally:
	return rc;
}

int evcpe_repo_to_param_attr_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_attr_list *list)
{
	int rc;
	struct evcpe_attr *attr;

	DEBUG("adding attribute to parameter attr list: %s", name);

	if ((rc = evcpe_repo_get_attr(repo, name, &attr))) {
		ERROR("failed to get attribute: %s", name);
		goto finally;
	}
	if ((rc = evcpe_attr_to_param_attr_list(attr, list))) {
		ERROR("failed to add attribute to param attr list: %s", name);
		goto finally;
	}

finally:
	return rc;
}
