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
#include "util.h"
#include "class.h"

#include "attr_schema.h"

evcpe_attr_schema * evcpe_attr_schema_new(evcpe_class *kls)
{
	evcpe_attr_schema *schema = (evcpe_attr_schema *)calloc(1,
			sizeof(evcpe_attr_schema));

	if (!schema) return NULL;

	schema->owner = kls;

	return schema;
}

void evcpe_attr_schema_free(evcpe_attr_schema *schema)
{
	if (!schema) return;

	TRACE("destructing attribute schema: %s", schema->name);

	if (schema->class) evcpe_class_free(schema->class);
	if (schema->name) free(schema->name);
	if (schema->value) free(schema->value);
	if (schema->number) free(schema->number);
	if (schema->constraint) evcpe_constraint_free(schema->constraint);
	if (schema->plugin) evcpe_plugin_unref(schema->plugin);
	free(schema);
}

int evcpe_attr_schema_set_name(evcpe_attr_schema *schema,
		const char *name, unsigned len)
{
	DEBUG("setting name: %.*s", len, name);
	return !(schema->name = evcpe_strdup(name, len));
}

int evcpe_attr_schema_set_type(evcpe_attr_schema *schema,
		evcpe_type_t type)
{
	TRACE("setting type: %d", type);

	if ((type == EVCPE_TYPE_OBJECT || type == EVCPE_TYPE_MULTIPLE) &&
			!(schema->class = evcpe_class_new(schema->name))) {
		ERROR("failed to create evcpe_class");
		return ENOMEM;
	}
	schema->type = type;
	return 0;
}

int evcpe_attr_schema_set_default(evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	int rc = 0;
	TRACE("setting default: %.*s", len, value);

	if (schema->type == EVCPE_TYPE_DATETIME) {
		if (evcpe_strncmp("auto", value, len)) {
			ERROR("only \"auto\" is accepted for default dateTime value");
			return EINVAL;
		}
	} else if ((rc = evcpe_type_validate(schema->type, value, len,
			schema->constraint, schema->pattern))) {
		ERROR("invalid default value: %.*s", len, value);
		return rc;
	}

	return !(schema->value = evcpe_strdup(value, len));
}

int evcpe_attr_schema_set_number(evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	evcpe_attr_schema *find = NULL;

	TRACE("setting number of entries: %.*s", len, value);

	if (schema->type != EVCPE_TYPE_MULTIPLE) {
		ERROR("number of entries is not applicable to type: %d", schema->type);
		return EPROTO;
	}
	if (!(find = evcpe_class_find(schema->owner, value, len))) {
		ERROR("number of entries attribute doesn't exist: %.*s", len, value);
		return EPROTO;
	}
	if (find->type != EVCPE_TYPE_UNSIGNED_INT) {
		ERROR("number of entries attribute is not unsigned int: %.*s", len,
				value);
		return EPROTO;
	}

	return !(schema->number = evcpe_strdup(value, len));
}

int evcpe_attr_schema_set_extension(evcpe_attr_schema *schema, int val)
{
	if (!schema) return EINVAL;

	schema->extension = !!val;

	return 0;
}

int evcpe_attr_schema_set_write(evcpe_attr_schema *schema, int write) {
	if (!schema) return EINVAL;
	schema->write = !!write;
	return 0;
}

int evcpe_attr_schema_set_notification(evcpe_attr_schema *schema,
		const char *value, unsigned len) {
	if (!schema) return EINVAL;

	if (schema->type == EVCPE_TYPE_OBJECT ||
		schema->type == EVCPE_TYPE_MULTIPLE) {
		ERROR("'notification' is not applicable to type: %d", schema->type);
		return EPROTO;
	}

	if (!evcpe_strncmp("passive", value, len)) {
		schema->notification = EVCPE_NOTIFICATION_PASSIVE;
	} else if (!evcpe_strncmp("active", value, len)) {
		schema->notification = EVCPE_NOTIFICATION_ACTIVE;
	} else if (evcpe_strncmp("false", value, len)) {
		ERROR("'notification' value must be (normal|passive|active): %.*s",
				len, value);
		return EPROTO;
	}

	return 0;
}

int evcpe_attr_schema_set_constraint(evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	int rc = 0;
	long val = 0;
	const char *start = NULL, *end = NULL;
	evcpe_attr_schema *find = NULL;
	evcpe_constraint *constraint = NULL;

	TRACE("setting constraint: %.*s", len, value);

	constraint = evcpe_constraint_new();
	if (!constraint) return ENOMEM;

	switch(schema->type) {
	case EVCPE_TYPE_STRING:
		if (!evcpe_atol(value, len, &val)) {
			if (val < 0) {
				ERROR("size constraint should not be negative: %ld", val);
				rc = EPROTO; goto finally;
			}

			evcpe_constraint_set_size(constraint, (unsigned)val);
		} else {
			if ((rc = evcpe_constraint_set_enums(constraint, value, len)))
				goto finally;
		}
		break;

	case EVCPE_TYPE_BASE64:
		if ((rc = evcpe_atol(value, len, &val))) {
			ERROR("invalid constraint: %.*s", len, value);
			rc = EPROTO; goto finally;
		}

		if (val < 0) {
			ERROR("size constraint should not be negative: %ld", val);
			rc = EPROTO; goto finally;
		}

		evcpe_constraint_set_size(constraint, val);
		schema->pattern = strdup(
			"^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$");
		break;

	case EVCPE_TYPE_INT:
	case EVCPE_TYPE_UNSIGNED_INT:
	case EVCPE_TYPE_UNSIGNED_LONG:
		for (start = value; start < value + len; start++) {
			if (*start == ':') {
				end = start;
				break;
			}
		}

		if (!end) {
			ERROR("constraint of int/unsignedInt should be "
							"a colon separated integer pair");
			rc = EPROTO; goto finally;
		}

		if (end == value) {
			if ((rc = evcpe_constraint_set_max(constraint, value + 1,
					len - 1))) {
				ERROR("failed to set max constraint");
				goto finally;
			}
		} else if (end == value + len - 1) {
			if ((rc = evcpe_constraint_set_min(constraint, value,
					end - value))) {
				ERROR("failed to set min constraint");
				goto finally;
			}
		} else {
			if ((rc = evcpe_constraint_set_range(constraint, value,
					end - value, end + 1, len - (end - value + 1)))) {
				ERROR("failed to set range constraint");
				goto finally;
			}
		}
		break;

	case EVCPE_TYPE_MULTIPLE:
		if (!(find = evcpe_class_find(schema->owner, value, len))) {
			ERROR("constraint attribute doesn't exist: %.*s", len, value);
			rc = EPROTO; goto finally;
		}

		if (find->type != EVCPE_TYPE_UNSIGNED_INT) {
			ERROR("constraint attribute is not unsigned int: %.*s", len, value);
			rc = EPROTO; goto finally;
		}

		if ((rc = evcpe_constraint_set_attr(constraint, value, len))) {
			ERROR("failed to set attribute constraint");
			goto finally;
		}
		break;

	default:
		ERROR("constraint is not applicable to type: %d", schema->type);
		rc = EPROTO; goto finally;
	}

	if (schema->constraint)
		evcpe_constraint_free(schema->constraint);
	schema->constraint = constraint;

	return 0;

finally:
	evcpe_constraint_free(constraint);
	return rc;
}

int evcpe_attr_schema_set_pattern(evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	if (!schema || !value || !len) return EPROTO;
	// TODO: Add support for regex
	return !(schema->pattern = evcpe_strdup(value, len));
}
