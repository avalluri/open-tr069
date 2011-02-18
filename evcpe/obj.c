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

struct evcpe_obj *evcpe_obj_new(struct evcpe_class *class,
		struct evcpe_attr *owner)
{
	struct evcpe_obj *obj;

	evcpe_trace(__func__, "constructing evcpe_obj: %s", class->name);

	if (!(obj = calloc(1, sizeof(struct evcpe_obj)))) {
		evcpe_error(__func__, "failed to calloc evcpe_obj");
		return NULL;
	}
	obj->class = class;
	obj->owner = owner;
	RB_INIT(&obj->attrs);
	return obj;
}

void evcpe_obj_free(struct evcpe_obj *obj)
{
	struct evcpe_attr *attr;

	if (!obj) return;

	evcpe_trace(__func__, "destructing evcpe_obj: %s", obj->class->name);

	while((attr = RB_MIN(evcpe_attrs, &obj->attrs))) {
		RB_REMOVE(evcpe_attrs, &obj->attrs, attr);
		evcpe_attr_unset(attr);
		switch(attr->schema->type) {
		case EVCPE_TYPE_OBJECT:
			if (attr->value.object) evcpe_obj_free(attr->value.object);
			break;
		case EVCPE_TYPE_MULTIPLE:
			break;
		default:
			evcpe_access_list_clear(&attr->value.simple.access_list);
			break;
		}
		if (attr->path) free(attr->path);
		free(attr);
	}
	if (obj->path) free(obj->path);
	free(obj);
}

int evcpe_obj_init(struct evcpe_obj *obj)
{
	int rc;
	struct evcpe_attr_schema *schema;
	struct evcpe_attr *attr;

	evcpe_trace(__func__, "initializing object: %s", obj->class->name);

	if (obj->owner) {
		obj->pathlen = snprintf(buffer, sizeof(buffer),
				"%s", obj->owner->path);
		if (obj->owner->schema->type == EVCPE_TYPE_MULTIPLE)
			obj->pathlen += snprintf(buffer + obj->pathlen,
					sizeof(buffer) - obj->pathlen,
					"%d.", obj->index + 1);
		if (obj->pathlen >= sizeof(buffer)) {
			evcpe_error(__func__, "object path exceeds limit: %s",
					obj->owner->schema->name);
			rc = EOVERFLOW;
			goto finally;
		}
	} else {
		buffer[0] = '\0';
	}
	if (!(obj->path = strdup(buffer))) {
		evcpe_error(__func__, "failed to strdup path: %s",
				buffer);
		rc = ENOMEM;
		goto finally;
	}

	TAILQ_FOREACH(schema, &obj->class->attrs, entry) {
		evcpe_debug(__func__, "adding attribute: %s", schema->name);
		if ((attr = evcpe_obj_find(obj, schema->name,
				strlen(schema->name)))) {
			evcpe_error(__func__, "duplicated attribute in %s: %s",
					obj->class->name, schema->name);
			rc = EINVAL;
			goto finally;
		}
		if (!(attr = calloc(1, sizeof(struct evcpe_attr)))) {
			evcpe_error(__func__, "failed to calloc evcpe_attr");
			rc = ENOMEM;
			goto finally;
		}
		attr->owner = obj;
		attr->schema = schema;
		if ((rc = evcpe_attr_init(attr))) {
			evcpe_error(__func__, "failed to init attribute of %s: %s",
					obj->class->name, schema->name);
			goto finally;
		}
		RB_INSERT(evcpe_attrs, &obj->attrs, attr);
	}
	rc = 0;

finally:
	return rc;
}

struct evcpe_attr *evcpe_obj_find(struct evcpe_obj *obj,
		const char *name, unsigned len)
{
	struct evcpe_attr find;
	struct evcpe_attr_schema schema;

	if (len >= sizeof(buffer)) {
		evcpe_error(__func__, "attribute name exceeds limit: %d >= %d",
				len, sizeof(buffer));
		return NULL;
	}

	evcpe_trace(__func__, "finding attribute: %.*s", len, name);

	memcpy(buffer, name, len);
	buffer[len] = '\0';
	schema.name = buffer;
	find.schema = &schema;
	return RB_FIND(evcpe_attrs, &obj->attrs, &find);
}

int evcpe_obj_get(struct evcpe_obj *obj,
		const char *name, unsigned len, struct evcpe_attr **attr)
{
	if (!obj || !name || !len) return EINVAL;

	evcpe_debug(__func__, "getting attribute: %.*s", len, name);

	if (!(*attr = evcpe_obj_find(obj, name, len))) {
		evcpe_error(__func__, "attribute of %s doesn't exist: %.*s",
				obj->class->name, len, name);
		return EVCPE_CPE_INVALID_PARAM_NAME;
	} else {
		return 0;
	}
}

int evcpe_obj_set_int(struct evcpe_obj *obj,
		const char *name, unsigned len, long value)
{
	int rc;
	char size[16];
	struct evcpe_attr *attr;

	evcpe_trace(__func__, "setting int value to %s: %.*s=%ld",
			obj->class->name, len, name, value);

	if ((rc = evcpe_obj_get(obj, name, len, &attr))) {
		evcpe_error(__func__, "failed to get attribute of %s: %.*s",
				obj->class->name, len, name);
		goto finally;
	}
	snprintf(size, sizeof(size), "%ld", value);
	if ((rc = evcpe_attr_set(attr, size, strlen(size)))) {
		evcpe_error(__func__, "failed to set attributeof %s: %.*s",
				obj->class->name, len, name);
		goto finally;
	}

finally:
	return rc;
}
