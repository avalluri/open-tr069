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
#include "util.h"
#include "obj.h"
#include "fault.h"

#include "attr.h"

static char buffer[257];

int evcpe_obj_set_int(struct evcpe_obj *obj,
		const char *name, unsigned len, long value);

int evcpe_attr_cmp(struct evcpe_attr *a, struct evcpe_attr *b)
{
	return strcmp(a->schema->name, b->schema->name);
}

RB_GENERATE(evcpe_attrs, evcpe_attr, entry, evcpe_attr_cmp);

int evcpe_attr_init(struct evcpe_attr *attr)
{
	int rc = 0;
	struct evcpe_attr *cons_attr = NULL;
	char *attr_name = NULL;
	const char *value = NULL;
	unsigned int len = 0;
	long val = 0;
	struct evcpe_obj *temp = NULL;

	TRACE("initializing attribute: %s", attr->schema->name);

	if ((attr->pathlen = snprintf(buffer, sizeof(buffer), "%s%s",
			attr->owner->path, attr->schema->name)) >= sizeof(buffer)) {
		rc = EOVERFLOW; goto finally;
	}
	if (attr->schema->type == EVCPE_TYPE_MULTIPLE ||
			attr->schema->type == EVCPE_TYPE_OBJECT)
		attr->pathlen += snprintf(buffer + attr->pathlen,
				sizeof(buffer) - attr->pathlen, ".");
	if (!(attr->path = strdup(buffer))) {
		rc = ENOMEM; goto finally;
	}
	switch (attr->schema->type) {
	case EVCPE_TYPE_MULTIPLE:
		if (!evcpe_constraint_get_attr(attr->schema->constraint, &attr_name)) {
			if ((rc = evcpe_obj_get(attr->owner, attr_name, strlen(attr_name),
					&cons_attr))) {
				ERROR("failed to get attribute: %s", attr_name);
				free(attr_name);
				rc = EINVAL; goto finally;
			}
			free(attr_name);

			if ((rc = evcpe_attr_get(cons_attr, &value, &len))) {
				ERROR("failed to get attribute value: %s", attr_name);
				rc = EINVAL; goto finally;
			}
			if (value) {
				if ((rc = evcpe_atol(value, strlen(value), &val))) {
					ERROR("invalid constraint value: %s", value);
					goto finally;
				}
				attr->value.multiple.max = val;
			}
		}
		if (attr->schema->number && (rc = evcpe_obj_set_int(attr->owner,
				attr->schema->number, strlen(attr->schema->number), 0))) {
			ERROR("failed to set number of entries attribute: "
					"%s", attr->schema->number);
			goto finally;
		}
		attr->value.multiple.list = tqueue_new(NULL,
				(tqueue_free_func)evcpe_obj_free);
		break;

	case EVCPE_TYPE_OBJECT:
		if (!(temp = evcpe_obj_new(attr->schema->class, attr))) {
			ERROR("failed to create evcpe_obj");
			rc = ENOMEM;
			goto finally;
		}
		if ((rc = evcpe_obj_init(temp))) {
			ERROR("failed to init object: %s", attr->schema->name);
			evcpe_obj_free(temp);
			goto finally;
		}
		attr->value.object = temp;
		break;

	default:
		evcpe_access_list_init(&attr->value.simple.access_list);
		if (attr->schema->value) {
			if ((rc = evcpe_attr_set(attr, attr->schema->value,
					strlen(attr->schema->value)))) {
				ERROR("failed set default value: %s = %s",
						attr->schema->name, attr->schema->value);
				goto finally;
			}
		}
	}

finally:
	return rc;
}

void evcpe_attr_set_cb(struct evcpe_attr *attr, evcpe_attr_cb cb, void *arg)
{
	DEBUG("setting callback on attribute: %s", attr->path);
	attr->cb = cb;
	attr->cbarg = arg;
}

int evcpe_attr_set_notification(struct evcpe_attr *attr,
		enum evcpe_notification notification)
{
	TRACE("setting notification on %s: %d",
			attr->schema->name, notification);

	if ((attr->schema->type == EVCPE_TYPE_OBJECT ||
			attr->schema->type == EVCPE_TYPE_MULTIPLE)) {
		ERROR("not a simple attribute: %s", attr->schema->name);
		return EINVAL;
	}
	if (notification < EVCPE_NOTIFICATION_OFF ||
			notification > EVCPE_NOTIFICATION_ACTIVE) {
		ERROR("notification value is out of range: %d", notification);
		return EPROTO;
	}
	attr->value.simple.notification = notification;

	return 0;
}

void evcpe_attr_unset(struct evcpe_attr *attr)
{
	TRACE("unsetting attribute: %s", attr->schema->name);

	switch(attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if (attr->value.object) {
			evcpe_obj_free(attr->value.object);
			attr->value.object = NULL;
		}
		break;
	case EVCPE_TYPE_MULTIPLE:
		tqueue_free(attr->value.multiple.list);
		attr->value.multiple.size = 0;
		break;
	default:
		if (attr->value.simple.string) {
			free(attr->value.simple.string);
			attr->value.simple.string = NULL;
		}
		break;
	}
}

int evcpe_attr_set(struct evcpe_attr *attr, const char *value, unsigned len)
{
	int rc = 0;

	if (!attr || !value) return EINVAL;

	TRACE("setting simple value to %s: %.*s", attr->schema->name, len, value);

	if (attr->schema->type == EVCPE_TYPE_OBJECT
			|| attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		ERROR("not simple type: %s (%s)", attr->schema->name,
				evcpe_type_to_str(attr->schema->type));
		return EVCPE_CPE_INVALID_PARAM_TYPE;
	}
	if ((rc = evcpe_type_validate(attr->schema->type, value, len,
			attr->schema->constraint, attr->schema->pattern))) {
		ERROR("not a valid value for %s (%s): %.*s", attr->schema->name,
				evcpe_type_to_str(attr->schema->type), len, value);
		return EVCPE_CPE_INVALID_PARAM_VALUE;
	}
	if (attr->schema->setter) {
		if ((rc = (*attr->schema->setter)(attr, value, len))) {
			ERROR("failed to set value by setter: %s => %.*s",
					attr->path, len, value);
			goto finally;
		}
	} else if(attr->schema->plugin &&
			  attr->schema->plugin->set_parameter_value) {
		if ((rc = attr->schema->plugin->set_parameter_value(
				attr->schema->plugin, attr->schema->name, value, len))) {
			/* plugin set_value failed, so saving it locally */
			WARN("Failed to set attribute value : %s => %.*s",
					attr->schema->name, len, value);

		} else return 0;
	}

	if (attr->value.simple.string) free(attr->value.simple.string);
	if ((rc = evcpe_strdup(value, len, &attr->value.simple.string))) {
		ERROR("failed to duplicate value: %.*s", len, value);
		goto finally;
	}

	DEBUG("%s: %.*s", attr->path, len, value);

	if (attr->cb) (*attr->cb)(attr, EVCPE_ATTR_EVENT_PARAM_SET,
				attr->value.simple.string, attr->cbarg);

finally:
	return rc;
}

int evcpe_attr_get(struct evcpe_attr *attr, const char **value, unsigned int *len)
{
	int rc = 0;

	TRACE("getting simple value of %s", attr->schema->name);

	*value = NULL;
	*len = 0;
	if (attr->schema->type == EVCPE_TYPE_OBJECT
			|| attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		ERROR("attr is not simple type");
		return EINVAL;
	}
	if (attr->schema->getter) {
		if ((rc = (*attr->schema->getter)(attr, value, len))) {
			ERROR("failed to get value by getter: %s", attr->path);
			goto finally;
		}
	} else if (attr->schema->plugin &&
			   attr->schema->plugin->get_paramter_value) {
		if ((rc = attr->schema->plugin->get_paramter_value(
				attr->schema->plugin, attr->schema->name, value, len)))
				return rc;
	}
	/* fallback to default */
	if (!*value) *value = attr->value.simple.string;
	*len = *value ? strlen(*value) : 0;

finally:
	return rc;
}

int evcpe_attr_get_obj(struct evcpe_attr *attr, struct evcpe_obj **child)
{
	TRACE("getting object value of %s", attr->schema->name);

	if (attr->schema->type != EVCPE_TYPE_OBJECT) {
		ERROR("attr is not object type");
		return EINVAL;
	} else {
		*child = attr->value.object;
		return 0;
	}
}

int evcpe_attr_add_obj(struct evcpe_attr *attr,
		struct evcpe_obj **child, unsigned int *index)
{
	struct tqueue_element *iter = NULL, *item = NULL;
	struct evcpe_obj* obj = NULL;
	int rc = 0;

	if (!attr) return EINVAL;

	TRACE("adding object to %s", attr->schema->name);

	if (attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("attr is not multiple object type");
		rc = EINVAL;
		goto finally;
	}
	if (attr->value.multiple.max &&
			attr->value.multiple.size >= attr->value.multiple.max) {
		ERROR("number of objects has reached constraint: %d",
				attr->value.multiple.max);
		rc = EVCPE_CPE_RESOUCES_EXCEEDS;
		goto finally;
	}
	item = NULL;
	*index = 0;
	TQUEUE_FOREACH(iter, attr->value.multiple.list) {
		if (!iter->data) {
			item = iter;
			break;
		}
		(*index) ++;
	}
	if (!item && !(item = tqueue_insert(attr->value.multiple.list, NULL))) {
		ERROR("failed to calloc evcpe_obj_item");
		rc = ENOMEM;
		goto finally;
	}

	if (!(item->data = obj = evcpe_obj_new(attr->schema->class, attr))) {
		ERROR("failed to create evcpe_obj");
		rc = ENOMEM;
		goto finally;
	}
	obj->index = *index;
	if ((rc = evcpe_obj_init(obj))) {
		ERROR("failed to init obj: %s", attr->schema->name);
		goto finally;
	}
	attr->value.multiple.size ++;
	if (attr->schema->number &&
		(rc = evcpe_obj_set_int(attr->owner, attr->schema->number,
				strlen(attr->schema->number), attr->value.multiple.size))) {
		ERROR("failed to set number of entries attribute: %s",
				attr->schema->number);
		goto finally;
	}
	*child = obj;
	if (attr->cb)
		(*attr->cb)(attr, EVCPE_ATTR_EVENT_OBJ_ADDED, obj, attr->cbarg);

finally:
	return rc;
}

int evcpe_attr_idx_obj(struct evcpe_attr *attr,
		unsigned int index, struct evcpe_obj **child)
{
	struct tqueue_element* item = NULL;
	int rc = 0;

	if (!attr) return EINVAL;

	TRACE("getting object from %s by index: %d", attr->schema->name, index);

	if (attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("attr is not multiple object type");
		rc = EVCPE_CPE_INVALID_PARAM_NAME;
		goto finally;
	}
	item = tqueue_nth_element(attr->value.multiple.list, index);
	if (!item || !item->data) {
		ERROR("multiple object attr doesn't exist: %d", index);
		rc = EVCPE_CPE_INVALID_PARAM_NAME;
		goto finally;
	}
	*child = (struct evcpe_obj*)item->data;
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_del_obj(struct evcpe_attr *attr, unsigned int index)
{
	struct evcpe_obj* obj = NULL;
	struct tqueue_element* item = NULL;
	int rc = 0;

	if (!attr) return EINVAL;

	DEBUG("deleting object from %s by index: %d", attr->schema->name, index);

	if (attr->schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("attr is not multiple object type");
		rc = EINVAL;
		goto finally;
	}
	item = tqueue_nth_element(attr->value.multiple.list, index);
	if (!item || !item->data) {
		ERROR("multiple object attr doesn't exist: %d", index);
		rc = EINVAL;
		goto finally;
	}
	attr->value.multiple.size --;
	obj = (struct evcpe_obj*)item->data;
	item->data = NULL;
	if (attr->cb)
		(*attr->cb)(attr, EVCPE_ATTR_EVENT_OBJ_DELETED, obj, attr->cbarg);
	evcpe_obj_free(obj);

finally:
	return rc;
}


int evcpe_attr_set_access_list(struct evcpe_attr *attr, const char *value,
		unsigned len) {
	if ((attr->schema->type == EVCPE_TYPE_OBJECT ||
		attr->schema->type == EVCPE_TYPE_MULTIPLE)) {
		ERROR("not a simple attribute: %s", attr->schema->name);
		return -1;
	}

	return evcpe_str_split(value, len, ',',
			(evcpe_str_split_cb)evcpe_access_list_add,
			&attr->value.simple.access_list);
}

struct _attr_param_value_list_data
{
	struct evcpe_obj *obj;
	struct evcpe_param_value_list *list;
};

static
int _schema_to_param_value_list(struct evcpe_attr_schema *schema, void *userdata)
{
	struct evcpe_attr *attr = NULL;
	struct _attr_param_value_list_data *data = userdata;
	int rc = 0;

	if ((rc = evcpe_obj_get(data->obj, schema->name, strlen(schema->name),
			&attr))) {
		ERROR("failed to get attribute: %s", schema->name);
		return rc;
	}
	if ((rc = evcpe_attr_to_param_value_list(attr, data->list))) {
		ERROR("failed to add attribute to param value list: %s", schema->name);
		return rc;
	}

	return 0;
}

static
int evcpe_attr_obj_to_param_value_list(struct evcpe_class *class,
		struct evcpe_obj *obj, struct evcpe_param_value_list *list)
{
	struct _attr_param_value_list_data userdata;
	userdata.obj = obj;
	userdata.list = list;

	DEBUG("adding object to param list: %s", obj->path);

	return tqueue_foreach(class->attrs,
			(tqueue_foreach_func)_schema_to_param_value_list, &userdata);
}

int evcpe_attr_to_param_value_list(struct evcpe_attr *attr,
		struct evcpe_param_value_list *list)
{
	int rc = 0;
	struct tqueue_element *item = NULL;
	struct evcpe_param_value *param = NULL;
	const char *value = NULL;
	unsigned int len = 0;

	if (attr->schema->extension) return 0;

	DEBUG("adding attribute to param value list: %s", attr->schema->name);

	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if ((rc = evcpe_attr_obj_to_param_value_list(attr->schema->class,
				attr->value.object, list))) {
			ERROR("failed to add to param list: %s", attr->value.object->path);
			goto finally;
		}
		break;
	case EVCPE_TYPE_MULTIPLE:
		TQUEUE_FOREACH(item, attr->value.multiple.list) {
			struct evcpe_obj *obj = NULL;
			if (!(obj = item->data)) continue;
			if ((rc = evcpe_attr_obj_to_param_value_list(attr->schema->class,
					obj, list))) {
				ERROR("failed to add to param list: %s", obj->path);
				goto finally;
			}
		}
		break;
	default:
		if ((rc = evcpe_param_value_list_add(list, &param,
				attr->path, attr->pathlen))) {
			ERROR("failed to add param value");
			goto finally;
		}
		if ((rc = evcpe_attr_get(attr, &value, &len))) {
			ERROR("failed to get attribute value: %s", attr->path);
			evcpe_param_value_list_remove(list, param);
			goto finally;
		}
		if ((rc = evcpe_param_value_set(param, value, len))) {
			ERROR("failed to set param value: %s => %.*s",
					attr->path, len, value);
			evcpe_param_value_list_remove(list, param);
			goto finally;
		}
		param->type = attr->schema->type;
		break;
	}

finally:
	return rc;
}

int evcpe_attr_to_param_info_list(struct evcpe_attr *attr,
		struct evcpe_param_info_list *list, int next_level)
{
	int rc = 0, len = 0;
	struct evcpe_param_info *param = NULL;
	struct evcpe_attr *child = NULL;
	struct tqueue_element *elm = NULL;

	if (attr->schema->extension) return 0;

	DEBUG("adding attribute to param info list: %s", attr->schema->name);

	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if (!next_level) {
			if ((rc = evcpe_param_info_list_add(list, &param,
					attr->value.object->path, attr->value.object->pathlen,
					attr->schema->write))) {
				ERROR("failed to add param info");
				goto finally;
			}
		}
		TQUEUE_FOREACH(elm, attr->schema->class->attrs) {
			struct evcpe_attr_schema* schema =
					(struct evcpe_attr_schema*)elm->data;
			if ((rc = evcpe_obj_get(attr->value.object,
					schema->name, strlen(schema->name), &child))) {
				ERROR("failed to get child attribute: %s",	schema->name);
				goto finally;
			}
			if (next_level) {
				if (schema->type == EVCPE_TYPE_OBJECT ||
						schema->type == EVCPE_TYPE_MULTIPLE) {
					len = snprintf(buffer, sizeof(buffer), "%s%s.",
							attr->value.object->path, schema->name);
					if ((rc = evcpe_param_info_list_add(list, &param,
							buffer, len, schema->write == 'W' ? 1 : 0))) {
						ERROR("failed to add param info");
						goto finally;
					}
				} else {
					if ((rc = evcpe_param_info_list_add(list, &param,
							child->path, child->pathlen, schema->write))) {
						ERROR("failed to add param info");
						goto finally;
					}
				}
			} else if ((rc = evcpe_attr_to_param_info_list(child, list, next_level))) {
				ERROR("failed to add child object to param info list: %s",
						schema->name);
				goto finally;
			}
		}
		break;
	case EVCPE_TYPE_MULTIPLE:
		if ((rc = evcpe_param_info_list_add(list, &param,
				attr->path, attr->pathlen,
				attr->schema->write == 'W' ? 1 : 0))) {
			ERROR("failed to add param info");
			goto finally;
		}
		if (next_level)
			break;
		TQUEUE_FOREACH(elm, attr->value.multiple.list) {
			struct evcpe_obj *obj = NULL;
			struct tqueue_element *attr_elm = NULL;
			if (!(obj = elm->data)) continue;
			if ((rc = evcpe_param_info_list_add(list, &param, obj->path,
					obj->pathlen, attr->schema->write))) {
				ERROR("failed to add param info");
				goto finally;
			}
			TQUEUE_FOREACH(attr_elm, attr->schema->class->attrs) {
				struct evcpe_attr_schema *schema = attr_elm->data;
				if ((rc = evcpe_obj_get(obj, schema->name,
						strlen(schema->name), &child))) {
					ERROR("failed to get child attribute: %s", schema->name);
					goto finally;
				}
				if ((rc = evcpe_attr_to_param_info_list(child, list, next_level))) {
					ERROR("failed to add child object "
							"to param info list: %s", schema->name);
					goto finally;
				}
			}
		}
		break;
	default:
		if ((rc = evcpe_param_info_list_add(list, &param, attr->path,
				strlen(attr->path), attr->schema->write))) {
			ERROR("failed to add param info");
			goto finally;
		}
		break;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_obj_to_param_attr_list(struct evcpe_attr_schema *schema,
		struct evcpe_obj *obj, struct evcpe_param_attr_list *list)
{
	int rc;
	struct evcpe_attr *child;

	DEBUG("adding object attribute to param list: %s", schema->name);

	if ((rc = evcpe_obj_get(obj, schema->name, strlen(schema->name), &child))) {
		ERROR("failed to get child attribute: %s", schema->name);
		goto finally;
	}
	if (!schema->extension && schema->type != EVCPE_TYPE_OBJECT &&
			schema->type != EVCPE_TYPE_MULTIPLE) {
		if ((rc = evcpe_attr_to_param_attr_list(child, list))) {
			ERROR("failed to add child attribute "
					"to param value list: %s", schema->name);
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

struct _attr_to_param_value_list_data
{
	struct evcpe_attr *attr;
	struct evcpe_param_attr_list *list;
};

static
int _attr_to_param_value_list(struct evcpe_attr_schema *schema, void *userdata)
{
	int rc = 0;
	struct _attr_to_param_value_list_data *data = userdata;

	if (schema->extension) return 0;

	if ((rc = evcpe_attr_obj_to_param_attr_list(schema,
			data->attr->value.object, data->list))) {
		ERROR("failed to add child object to param list: %s", schema->name);
	}

	return rc;
}

int evcpe_attr_to_param_attr_list(struct evcpe_attr *attr,
		struct evcpe_param_attr_list *list)
{
	int rc;
	struct tqueue_element *item;
	struct evcpe_attr_schema *schema;
	struct evcpe_param_attr *param;
	struct _attr_to_param_value_list_data userdata;

	if (attr->schema->extension) return 0;

	DEBUG("adding attribute to param value list: %s", attr->schema->name);

	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		userdata.attr = attr;
		userdata.list = list;
		tqueue_foreach(attr->schema->class->attrs,
				(tqueue_foreach_func)_attr_to_param_value_list, &userdata);

		break;
	case EVCPE_TYPE_MULTIPLE:
		TQUEUE_FOREACH(item, attr->value.multiple.list) {
			struct evcpe_obj* obj = NULL;
			if (!(obj = item->data)) continue;
			if ((rc = evcpe_attr_obj_to_param_attr_list(attr->schema, obj,
					list))) {
				ERROR("failed to add child object to param list: %s",
						attr->schema->name);
				goto finally;
			}
		}
		break;
	default:
		if ((rc = evcpe_param_attr_list_add(list, &param,
				attr->path, attr->pathlen))) {
			ERROR("failed to add param value");
			goto finally;
		}
		evcpe_access_list_clone(&attr->value.simple.access_list,
				&param->access_list, &rc);
		if (rc) {
			ERROR("failed to clone access list");
			evcpe_param_attr_list_remove(list, param);
			goto finally;
		}
		param->notification = attr->value.simple.notification;
		break;
	}
	rc = 0;

finally:
	return rc;
}

struct evcpe_attr *evcpe_attr_new(struct evcpe_obj *owner,
		struct evcpe_attr_schema *schema)
{
	struct evcpe_attr *attr = NULL;

	if (!(attr = (struct evcpe_attr *)calloc(1, sizeof(struct evcpe_attr)))) {
		return NULL;
	}

	attr->owner = owner;
	attr->schema = schema;

	return attr;
}

void evcpe_attr_free(struct evcpe_attr *attr)
{
	if (!attr) return;
	if (attr->path) free(attr->path);
	evcpe_attr_unset(attr);
	free(attr);
}

void evcpe_attrs_init (struct evcpe_attrs *attrs)
{
	if (!attrs) return;
	RB_INIT(attrs);
}

void evcpe_attrs_clear(struct evcpe_attrs *attrs)
{
	struct evcpe_attr *attr = NULL;

	if (!attrs) return;

	while((attr = RB_MIN(evcpe_attrs, attrs))) {
		RB_REMOVE(evcpe_attrs, attrs, attr);
		evcpe_attr_free(attr);
	}
}
