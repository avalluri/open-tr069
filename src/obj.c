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
#include "fault.h"

#include "obj.h"

static char buffer[257];

evcpe_obj *evcpe_obj_new(evcpe_class *class,
		evcpe_attr *owner)
{
	evcpe_obj *obj;

	TRACE("constructing evcpe_obj: %s", class->name);

	if (!(obj = calloc(1, sizeof(evcpe_obj)))) {
		ERROR("failed to calloc evcpe_obj");
		return NULL;
	}
	obj->class = class;
	obj->owner = owner;
	evcpe_attrs_init(&obj->attrs);
	return obj;
}

void evcpe_obj_free(evcpe_obj *obj)
{
	if (!obj) return;

	TRACE("destructing evcpe_obj: %s", obj->class->name);

	evcpe_attrs_clear(&obj->attrs);
	if (obj->path) free(obj->path);
	free(obj);
}

int evcpe_obj_init(evcpe_obj *obj)
{
	int rc = 0;
	evcpe_attr *attr = NULL;
	tqueue_element* elm = NULL;

	TRACE("initializing object: %s", obj->class->name);

	if (obj->owner) {
		obj->pathlen = snprintf(buffer, sizeof(buffer), "%s", obj->owner->path);
		if (obj->owner->schema->type == EVCPE_TYPE_MULTIPLE)
			obj->pathlen += snprintf(buffer + obj->pathlen,
					sizeof(buffer) - obj->pathlen, "%d.", obj->index + 1);
		if (obj->pathlen >= sizeof(buffer)) {
			ERROR("object path exceeds limit: %s", obj->owner->schema->name);
			rc = EOVERFLOW;
			goto finally;
		}
	} else {
		buffer[0] = '\0';
	}
	if (!(obj->path = strdup(buffer))) {
		ERROR("failed to strdup path: %s", buffer);
		rc = ENOMEM;
		goto finally;
	}

	TQUEUE_FOREACH(elm, obj->class->attrs) {
		evcpe_attr_schema *schema = (evcpe_attr_schema *)elm->data;
		DEBUG("adding attribute: %s", schema->name);
		if ((attr = evcpe_obj_find(obj, schema))) {
			ERROR("duplicated attribute in %s: %s", obj->class->name,
					schema->name);
			rc = EINVAL;
			goto finally;
		}
		if (!(attr = evcpe_attr_new(obj, schema))) {
			ERROR("failed to calloc evcpe_attr");
			rc = ENOMEM;
			goto finally;
		}

		if ((rc = evcpe_attr_init(attr))) {
			ERROR("failed to init attribute of %s: %s", obj->class->name,
					schema->name);
			goto finally;
		}
		RB_INSERT(_evcpe_attrs, &obj->attrs, attr);
	}

finally:
	return rc;
}

evcpe_attr *evcpe_obj_find(evcpe_obj *obj, evcpe_attr_schema *schema)
{
	evcpe_attr find;

	TRACE("finding attribute: %s", schema->name);

	find.schema = schema;
	return RB_FIND(_evcpe_attrs, &obj->attrs, &find);
}

evcpe_attr * _find_attr_by_schema(evcpe_attr *attr,
		evcpe_attr_schema *schema) {
	evcpe_attr *tmp_attr = NULL;
	if (!attr) return NULL;

	if (attr->schema == schema) return attr;
	if (attr->schema->type == EVCPE_TYPE_OBJECT) {
		if ((tmp_attr = evcpe_obj_find_deep(attr->value.object, schema)) != NULL)
			return tmp_attr;
	} else if(attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		tqueue_element* item = NULL;
		TQUEUE_FOREACH(item, attr->value.multiple.list) {
			evcpe_obj *obj = (evcpe_obj*)item->data;
			if (obj && (tmp_attr = evcpe_obj_find_deep(obj, schema)) != NULL)
				return tmp_attr;
		}
	}

	if ((tmp_attr =_find_attr_by_schema(RB_RIGHT(attr, entry), schema)) != NULL)
		return tmp_attr;
	if ((tmp_attr =_find_attr_by_schema(RB_LEFT(attr, entry), schema)) != NULL)
		return tmp_attr;

	return NULL;
}

evcpe_attr * evcpe_obj_find_deep(evcpe_obj *obj,
		evcpe_attr_schema *schema)
{
  return _find_attr_by_schema(RB_ROOT(&obj->attrs), schema);
}

int evcpe_obj_get(evcpe_obj *obj, const char *name, unsigned len,
		evcpe_attr **attr)
{
	evcpe_attr_schema schema;
	if (!obj || !name || !len) return EINVAL;

	DEBUG("getting attribute: %.*s", len, name);
	if (len >= sizeof(buffer)) {
		WARN("attribute name exceeds limit: %u >= %lu", len, sizeof(buffer));
		return EVCPE_CPE_INVALID_PARAM_NAME;
	}

	memcpy(buffer, name, len);
	buffer[len] = '\0';
	schema.name = buffer;

	if (!(*attr = evcpe_obj_find(obj, &schema))) {
		WARN("attribute of %s doesn't exist: %.*s", obj->class->name, len,
				name);
		return EVCPE_CPE_INVALID_PARAM_NAME;
	}

	return 0;
}

int evcpe_obj_get_attr_value(evcpe_obj* obj, const char *attr_name,
		const char** value, unsigned* value_len)
{
	int rc = 0;
	evcpe_attr* attr = NULL;
	evcpe_attr_schema schema;

	schema.name = (char*)attr_name;
	if (!(attr = evcpe_obj_find(obj, &schema))) {
		ERROR("attribute of %s doesn't exist: %s", obj->class->name, attr_name);
		return EVCPE_CPE_INVALID_PARAM_NAME;
	}

	return evcpe_attr_get(attr, value, value_len);
}

int evcpe_obj_set_int(evcpe_obj *obj, const char *name, unsigned len,
		long value)
{
	int rc;
	char size[16];
	evcpe_attr *attr;

	TRACE("setting int value to %s: %.*s=%ld",
			obj->class->name, len, name, value);

	if ((rc = evcpe_obj_get(obj, name, len, &attr))) {
		ERROR("failed to get attribute of %s: %.*s",
				obj->class->name, len, name);
		goto finally;
	}
	snprintf(size, sizeof(size), "%ld", value);
	if ((rc = evcpe_attr_set(attr, size, strlen(size)))) {
		ERROR("failed to set attributeof %s: %.*s",
				obj->class->name, len, name);
		goto finally;
	}

finally:
	return rc;
}
