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
#include "download.h"

static int evcpe_repo_get_attr(evcpe_repo *repo, const char *name,
		evcpe_attr **attr);

static void evcpe_repo_set_obj_attr_cb(evcpe_repo *repo,
		evcpe_obj *obj);

static void evcpe_repo_attr_cb(evcpe_attr *attr,
		enum evcpe_attr_event event, int inform, void *data, void *cbarg);

static int _listener_cmp_cb(void *node_data, void* cb, void* userdata)
{
	return ((evcpe_repo_listener*)node_data)->cb != cb;
}

evcpe_repo *evcpe_repo_new(evcpe_obj *root)
{
	evcpe_repo *repo;

	DEBUG("constructing evcpe_repo");

	if (!(repo = calloc(1, sizeof(evcpe_repo)))) {
		ERROR("failed to calloc evcpe_repo");
		return NULL;
	}
	repo->listeners = tqueue_new((tqueue_compare_func_t)_listener_cmp_cb, free);
	repo->forced_inform_attrs = tqueue_new(NULL, NULL);
	repo->changed_atts = tqueue_new(NULL, NULL);
	repo->pending_events = evcpe_event_list_new();
	repo->root = root;
	repo->root_plugin = NULL;
	repo->active_downloads = tqueue_new(NULL, 
                  (tqueue_free_func_t)evcpe_download_free);

	evcpe_repo_init(repo);

	return repo;
}

void evcpe_repo_free(evcpe_repo *repo)
{
	if (!repo) return;

	TRACE("destructing repo");

	tqueue_free(repo->forced_inform_attrs);
	tqueue_free(repo->changed_atts);
	tqueue_free(repo->listeners);
	tqueue_free(repo->pending_events);
	evcpe_plugin_unref(repo->root_plugin);

	free(repo);
}

int evcpe_repo_init(evcpe_repo* repo) {

	tqueue_element* elm = NULL;
	const char* device_node = "InternetGatewayDevice";
	int         device_node_len = strlen(device_node);
	evcpe_attr_schema* device_schema = NULL;

	if (!(device_schema = evcpe_class_find(repo->root->class, device_node,
				device_node_len))) {
		ERROR("%s schema not found", device_node);
	} else if (device_schema->plugin)
		repo->root_plugin = evcpe_plugin_ref(device_schema->plugin);

	/* Gather all Force Inform attributes */
	TQUEUE_FOREACH(elm, repo->root->class->inform_attrs) {
		evcpe_attr_schema* schema =
				(evcpe_attr_schema *)elm->data;
		evcpe_attr* attr = NULL;
		/* sanity check */
		if (!schema->inform) continue;
		if (!(attr = evcpe_obj_find_deep(repo->root, schema))) {
			WARN("missing inform attribute '%s' in object", schema->name);
			continue;
		}

		tqueue_insert(repo->forced_inform_attrs, attr);
	}
#if 0
	TQUEUE_FOREACH(elm, repo->root->class->attrs) {
		evcpe_attr_schema* schema = elm->data;
		evcpe_attr* attr = NULL;

		if (schema->inform) {
			if (!(attr = evcpe_obj_find_deep(repo->root, schema))) {
				WARN("missing inform attribute '%s' in object", schema->name);
				continue;
			}
			tqueue_insert(repo->forced_inform_attrs, attr);
		} else if (schema->notification != EVCPE_NOTIFICATION_OFF &&
				   schema->plugin) {
			evcpe_plugin_set_value_change_listener(schema->plugin,
					schema->name, _repo_param_value_changed, repo);
			tqueue_insert(repo->changed_atts, attr);
		}
	}
#endif
	evcpe_repo_set_obj_attr_cb(repo, repo->root);

	return 0;
}

int evcpe_repo_listen(evcpe_repo *repo, evcpe_repo_listen_cb_t cb, void *arg)
{
	int rc;
	evcpe_repo_listener *listener;

	DEBUG("listening repository");

	if (!(listener = calloc(1, sizeof(evcpe_repo_listener)))) {
		ERROR("failed to calloc evcpe_repo_listener");
		rc = ENOMEM;
		goto finally;
	}
	listener->cb = cb;
	listener->cbarg = arg;
	tqueue_insert(repo->listeners, listener);
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_unlisten(evcpe_repo *repo, evcpe_repo_listen_cb_t cb)
{
	tqueue_element* node = NULL;

	DEBUG("unlistening repository");
	if (! repo || ! cb) return EINVAL;

	if((node = tqueue_find(repo->listeners, cb)))
		tqueue_remove(repo->listeners, node);

	return 0;
}

void evcpe_repo_set_download_cb(evcpe_repo* repo, evcpe_repo_download_cb_t cb,
		void* cb_data) {
	if (!repo) return;

	repo->download_cb = cb;
	repo->download_cb_data = cb_data;
}

static
int evcpe_repo_locate(evcpe_repo* repo, const char* name,
		evcpe_obj** obj, evcpe_attr** attr)
{
	int rc = 0;
	const char *start, *end;
	unsigned index;

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
			if (!(index = atoi(start)) && errno) {
				ERROR("failed to convert to integer: %.*s",
						(int)(end - start), start);
				return EVCPE_CPE_INVALID_PARAM_NAME;
			}
			if (index <= 0) {
				ERROR("invalid instance number: %d", index);
				return EVCPE_CPE_INVALID_PARAM_NAME;
			}
			if ((rc = evcpe_attr_idx_obj(*attr, index - 1, obj))) {
				ERROR("indexed object doesn't exist: [%d]", index - 1);
				return rc;
			}
		} else if ((rc = evcpe_obj_get(*obj, start, end - start, attr))) {
			ERROR("failed to get attribute: %.*s", (int)(end - start), start);
			return rc;
		} else if ((*attr)->schema->type == EVCPE_TYPE_MULTIPLE) {
		} else if ((*attr)->schema->type == EVCPE_TYPE_OBJECT) {
			*obj = (*attr)->value.object;
		} else {
			ERROR("not an object/multiple attribute: %s", (*attr)->schema->name);
			return EVCPE_CPE_INVALID_PARAM_NAME;
		}
		start = ++ end;
	}
	if (start != end) {
		if ((rc = evcpe_obj_get(*obj, start, end - start, attr))) {
			ERROR("failed to get attribute: %.*s", (int)(end - start), start);
			return rc;
		} else if ((*attr)->schema->type == EVCPE_TYPE_OBJECT ||
				(*attr)->schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("not a simple attribute: %.*s", (int)(end - start), start);
			return EVCPE_CPE_INVALID_PARAM_NAME;
		}
	}

	return 0;
}

int evcpe_repo_get_obj(evcpe_repo *repo, const char *name,
		evcpe_obj **obj)
{
	int rc = 0;
	evcpe_attr *attr = NULL;

	INFO("getting object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, obj, &attr))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr) {
		ERROR("object not found: %s", name);
		rc = EINVAL;
		goto finally;
	} else if (attr->schema->type == EVCPE_TYPE_OBJECT) {
	} else if (attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		if ((*obj)->class != attr->schema->class) {
			ERROR("missing instance number: %s", name);
			rc = EINVAL;
			goto finally;
		}
	} else {
		ERROR("not an object/multiple attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}

finally:
	return rc;
}

int evcpe_repo_get(evcpe_repo *repo, const char *name,
		const char **value, unsigned int *len)
{
	int rc = 0;
	evcpe_obj *obj = NULL;
	evcpe_attr *attr = NULL;

	DEBUG("getting parameter: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr))) {
		ERROR("failed to locate object: %s", name);
	}
	else if ((rc = evcpe_attr_get(attr, value, len))) {
		ERROR("failed to get value: %s", name);
		rc = EINVAL;
	}

	return rc;
}

int evcpe_repo_set(evcpe_repo *repo, const char *name,
		const char *value, unsigned int len)
{
	int rc = 0;
	evcpe_obj *obj = NULL;
	evcpe_attr *attr = NULL;

	DEBUG("setting parameter: %s => %.*s", name, len, value);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr->schema->write) return EVCPE_CPE_NON_WRITABLE_PARAM;
	if ((rc = evcpe_attr_set(attr, value, len))) {
		ERROR("failed to set value: %s", name);
		rc = EINVAL;
	}

finally:
	return rc;
}

int evcpe_repo_getcpy(evcpe_repo *repo, const char *name,
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

const char *evcpe_repo_find(evcpe_repo *repo, const char *name)
{
	evcpe_obj *obj;
	evcpe_attr *attr;
	unsigned int len;
	const char *value;

	DEBUG("finding parameter: %s", name);

	if (evcpe_repo_locate(repo, name, &obj, &attr))
		return NULL;
	if (evcpe_attr_get(attr, &value, &len))
		return NULL;
	return value;
}

int evcpe_repo_get_attr(evcpe_repo *repo, const char *name,
		evcpe_attr **attr)
{
	int rc;
	evcpe_obj *obj;

	DEBUG("getting attribute: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, attr))) {
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

int evcpe_repo_add_obj(evcpe_repo *repo, const char *name,
		unsigned int *index)
{
	int rc;
	evcpe_obj *obj = NULL;
	evcpe_attr *attr = NULL;

	DEBUG("adding object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr))) {
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

int evcpe_repo_del_obj(evcpe_repo *repo, const char *name)
{
	int rc;
	evcpe_obj *obj = NULL;
	evcpe_attr *attr = NULL;

	DEBUG("deleting object: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr))) {
		ERROR("failed to locate object: %s", name);
		goto finally;
	}
	if (!attr || attr->schema->type != EVCPE_TYPE_OBJECT) {
		ERROR("not an object attribute: %s", name);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_attr_del_obj(attr, obj->index))) {
		ERROR("failed to delete object: %s", name);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_get_objs(evcpe_repo *repo, const char *name,
		tqueue **list, unsigned int *size)
{
	int rc;
	evcpe_obj *obj;
	evcpe_attr *attr;

	DEBUG("getting multiple objects: %s", name);

	if ((rc = evcpe_repo_locate(repo, name, &obj, &attr))) {
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

void evcpe_repo_attr_cb(evcpe_attr *attr, enum evcpe_attr_event event,
		int inform_acs, void *data, void *cbarg)
{
	tqueue_element* node = NULL;
	evcpe_repo *repo = cbarg;

	if (event == EVCPE_ATTR_EVENT_OBJ_ADDED) {
		evcpe_repo_set_obj_attr_cb(repo, data);
	}
	TQUEUE_FOREACH(node, repo->listeners) {
		evcpe_repo_listener *listener = node->data;

		switch (event) {
		case EVCPE_ATTR_EVENT_OBJ_ADDED:
		case EVCPE_ATTR_EVENT_OBJ_DELETED:
			(*listener->cb)(repo, event,
					((evcpe_obj *)data)->path, listener->cbarg);
			break;
		case EVCPE_ATTR_EVENT_PARAM_SET:
			(*listener->cb)(repo, event, attr->path, listener->cbarg);
			if (inform_acs) {
				tqueue_insert(repo->changed_atts, attr);
				// TODO: Initial Inform
			}
			break;
		}
	}
}

void evcpe_repo_set_obj_attr_cb(evcpe_repo *repo,
		evcpe_obj *obj)
{
	tqueue_element *item = NULL;
	evcpe_attr *attr = NULL;

	DEBUG("setting callback on attributes of %s", obj->path);

	RB_FOREACH(attr, _evcpe_attrs, &obj->attrs) {
		/* TODO:We are interested in only attributes which are not maintained
		 * by plugin
		 */
		if (attr->schema->plugin) continue;
		evcpe_attr_set_cb(attr, evcpe_repo_attr_cb, repo);
		switch (attr->schema->type) {
		case EVCPE_TYPE_OBJECT:
			evcpe_repo_set_obj_attr_cb(repo, attr->value.object);
			break;
		case EVCPE_TYPE_MULTIPLE:
			TQUEUE_FOREACH(item, attr->value.multiple.list) {
				if (!item->data) continue;
				evcpe_repo_set_obj_attr_cb(repo, (evcpe_obj*)item->data);
			}
			break;
		default:
			break;
		}
	}
}

int evcpe_repo_add_event(evcpe_repo *repo,
		evcpe_event_code_t code, const char *command_key)
{
	DEBUG("adding event: %s - %s", evcpe_event_code_to_str(code), command_key);

	// FIXME: Let CPE know about this event, so that if not in session, then
	//         starts a new session to send this event to ACS

	return (evcpe_event_list_add(repo->pending_events, code,
			command_key ? command_key : "") == NULL) ? -1: 0;
}

int evcpe_repo_del_event(evcpe_repo *repo, evcpe_event_code_t code)
{
	tqueue_element* node = NULL;

	DEBUG("deleting event: %s", evcpe_event_code_to_str(code));

	if (!(node =  tqueue_find(repo->pending_events, (void*)code)))
		return -1;

	tqueue_remove(repo->pending_events, node);

	return 0;
}

void evcpe_repo_clear_pending_events(evcpe_repo* repo)
{
	if (!repo) return;

	TRACE("Clearning all pending events.");

	tqueue_remove_all(repo->pending_events);
}

static
int evcpe_repo_to_inform_param_value_list(evcpe_repo *repo, tqueue *list)
{
	int rc = 0;
	tqueue_element* elm = NULL;

	TQUEUE_FOREACH(elm, repo->forced_inform_attrs) {
		evcpe_attr *attr = (evcpe_attr*)elm->data;
		if ((rc = evcpe_attr_to_param_value_list(attr, list))) {
			ERROR("failed to add param to value list");
			return rc;
		}
	}

	return 0;
}

int evcpe_repo_to_inform(evcpe_repo *repo, evcpe_inform *inform)
{
	int rc = 0;
	tqueue *objs = NULL;
	tqueue_element *item = NULL;
	unsigned int count;
	evcpe_attr *attr;
	const char *code, *command;
	unsigned int len;
	struct evcpe_event *event;
	const char* value = NULL;
	evcpe_obj* devinfo_obj = NULL;

	DEBUG("filling inform request");

	if ((rc = evcpe_repo_get_obj(repo, ".DeviceInfo.", &devinfo_obj))) {
		ERROR("failed to get manufacturer");
		return rc;
	}
	if (!(rc = evcpe_obj_get_attr_value(devinfo_obj, "Manufacturer", &value,
			&len))) {
		snprintf(inform->device_id.manufacturer,
				sizeof(inform->device_id.manufacturer), "%.*s", len, value);
	} else {
		ERROR("failed to get manufacturer");
		goto finally;
	}

	if (!(rc = evcpe_obj_get_attr_value(devinfo_obj, "ManufacturerOUI", &value,
					&len))) {
		snprintf(inform->device_id.oui, sizeof(inform->device_id.oui),
				"%.*s", len, value);
	} else {
		ERROR("failed to get manufacturer OUI");
		goto finally;
	}

	if (!(rc = evcpe_obj_get_attr_value(devinfo_obj, "ProductClass", &value,
			&len))) {
		snprintf(inform->device_id.product_class,
				sizeof(inform->device_id.product_class), "%.*s", len, value);

	} else {
		ERROR("failed to get product class");
		goto finally;
	}

	if (!(rc = evcpe_obj_get_attr_value(devinfo_obj, "SerialNumber", &value,
			&len))) {
		snprintf(inform->device_id.serial_number,
			sizeof(inform->device_id.serial_number), "%.*s", len, value);
	} else {
		ERROR("failed to get serial number");
		goto finally;
	}

	if ((rc = evcpe_repo_getcpy(repo, ".Time.CurrentLocalTime",
			inform->current_time, sizeof(inform->current_time)))) {
		ERROR("failed to get current local time");
		goto finally;
	}

	inform->events = repo->pending_events;

	if ((rc = evcpe_repo_to_inform_param_value_list(repo,
			inform->parameter_list))) {
		ERROR("failed to add inform param value list");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_repo_to_param_value_list(evcpe_repo *repo, const char *name,
		tqueue *list)
{
	int rc;
	evcpe_attr *attr;

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

int evcpe_repo_to_param_info_list(evcpe_repo *repo, const char *name,
		tqueue *list, int next_level)
{
	int rc = 0;
	evcpe_param_info *param;
	evcpe_attr *attr;

	DEBUG("adding attribute to parameter info list: '%s', next_level:%d",
			name, next_level);

	if ((rc = evcpe_repo_get_attr(repo, *name == '\0' ? "." : name, &attr))) {
		ERROR("failed to get attribute: %s", name);
		goto finally;
	}
	if (next_level && attr->schema->type != EVCPE_TYPE_OBJECT &&
			attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		rc = EVCPE_CPE_INVALID_ARGUMENTS;
		goto finally;
	}

	// No idea why this speical condition.!!!
	if (*name == '\0') {
		if (!(param = evcpe_param_info_list_add(list,
				attr->value.object->path, attr->value.object->pathlen,
				attr->schema->write))) {
			ERROR("failed to add param info");
			goto finally;
		}
	} else	if ((rc = evcpe_attr_to_param_info_list(attr, list, next_level))) {
		ERROR("failed to add attribute to param info list: %s", name);
		goto finally;
	}

finally:
	return rc;
}

int evcpe_repo_to_param_attr_list(evcpe_repo *repo, const char *name,
		tqueue *list)
{
	int rc;
	evcpe_attr *attr;

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

int evcpe_repo_set_param_atts(evcpe_repo* repo, tqueue* list) {
	int rc = 0;
	tqueue_element* node = NULL;

	TQUEUE_FOREACH(node, list) {
		evcpe_set_param_attr* param = node->data;
		evcpe_attr* attr = NULL;
		if ((rc = evcpe_repo_get_attr(repo, param->info->name, &attr))) {
			return rc;
		}
		if (param->notification_change) {
			evcpe_attr_set_notification(attr, param->info->notification);
		}
		if (param->access_list_change) {
			evcpe_attr_set_access_list(attr, param->info->access_list);
		}
	}

	return 0;
}

static
void _download_state_change_cb(evcpe_download* details,
		evcpe_download_state_info* state_info, void* arg)
{
	evcpe_repo* repo = arg;

	if (state_info->state == EVCPE_DOWNLOAD_APPLIED ||
		state_info->state == EVCPE_DOWNLOAD_FAILED) {
		if (state_info->state == EVCPE_DOWNLOAD_APPLIED) {
			evcpe_repo_add_event(repo, EVCPE_EVENT_M_DOWNLOAD,
					details->command_key);

		}
		if (repo->download_cb)
			repo->download_cb(details, state_info, repo->download_cb_data);

		tqueue_remove_data(repo->active_downloads, details);
	}
}


int evcpe_repo_download(evcpe_repo* repo, evcpe_download* details)
{
	if(!repo->root_plugin || !repo->root_plugin->download) {
		ERROR("No plugin found to handle download");
		return EVCPE_CPE_INTERNAL_ERROR;
	}

	return repo->root_plugin->download(repo->root_plugin, details,
			_download_state_change_cb, repo);
}
