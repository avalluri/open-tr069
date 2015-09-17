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

	TRACE("constructing evcpe_obj: %s", class->name);

	if (!(obj = calloc(1, sizeof(struct evcpe_obj)))) {
		ERROR("failed to calloc evcpe_obj");
		return NULL;
	}
	obj->class = class;
	obj->owner = owner;
	evcpe_attrs_init(&obj->attrs);
	return obj;
}

void evcpe_obj_free(struct evcpe_obj *obj)
{
	if (!obj) return;

	TRACE("destructing evcpe_obj: %s", obj->class->name);

	evcpe_attrs_clear(&obj->attrs);
	if (obj->path) free(obj->path);
	free(obj);
}

int evcpe_obj_init(struct evcpe_obj *obj)
{
	int rc = 0;
	struct evcpe_attr_schema *schema = NULL;
	struct evcpe_attr *attr = NULL;

	TRACE("initializing object: %s", obj->class->name);

	if (obj->owner) {
		obj->pathlen = snprintf(buffer, sizeof(buffer), "%s", obj->owner->path);
		if (obj->owner->schema->type == EVCPE_TYPE_MULTIPLE)
			obj->pathlen += snprintf(buffer + obj->pathlen,
					sizeof(buffer) - obj->pathlen,
					"%d.", obj->index + 1);
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

	TAILQ_FOREACH(schema, &obj->class->attrs, entry) {
		DEBUG("adding attribute: %s", schema->name);
		if ((attr = evcpe_obj_find(obj, schema->name, strlen(schema->name)))) {
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
		RB_INSERT(evcpe_attrs, &obj->attrs, attr);
	}

finally:
	return rc;
}

struct evcpe_attr *evcpe_obj_find(struct evcpe_obj *obj,
		const char *name, unsigned len)
{
	struct evcpe_attr find;
	struct evcpe_attr_schema schema;

	if (len >= sizeof(buffer)) {
		ERROR("attribute name exceeds limit: %u >= %lu", len, sizeof(buffer));
		return NULL;
	}

	TRACE("finding attribute: %.*s", len, name);

	memcpy(buffer, name, len);
	buffer[len] = '\0';
	schema.name = buffer;
	find.schema = &schema;
	return RB_FIND(evcpe_attrs, &obj->attrs, &find);
}

int evcpe_obj_get(struct evcpe_obj *obj, const char *name, unsigned len,
		struct evcpe_attr **attr)
{
	if (!obj || !name || !len) return EINVAL;

	DEBUG("getting attribute: %.*s", len, name);

	if (!(*attr = evcpe_obj_find(obj, name, len))) {
		ERROR("attribute of %s doesn't exist: %.*s", obj->class->name, len,
				name);
		return EVCPE_CPE_INVALID_PARAM_NAME;
	}

	return 0;
}

int evcpe_obj_set_int(struct evcpe_obj *obj, const char *name, unsigned len,
		long value)
{
	int rc;
	char size[16];
	struct evcpe_attr *attr;

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
