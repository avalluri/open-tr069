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

#include <string.h>
#include <errno.h>

#include "log.h"
#include "util.h"
#include "xml.h"

#include "attr_xml.h"

static int evcpe_attr_to_xml_obj_param_name(struct evcpe_obj *obj,
		struct evbuffer *buffer);

static int evcpe_attr_to_xml_obj_param_names(struct evcpe_obj *obj,
		int next_level, struct evbuffer *buffer);

static int evcpe_attr_obj_to_xml(struct evcpe_attr *attr, struct evcpe_obj *obj,
		unsigned int indent, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_attr *child;
	struct evcpe_attr_schema *schema;

	if ((rc = evcpe_xml_add_indent(buffer, EVCPE_XML_INDENT, indent))) {
		evcpe_error(__func__, "failed to append indentation");
		goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", attr->schema->name))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}
	TAILQ_FOREACH(schema, &attr->schema->class->attrs, entry) {
		if ((rc = evcpe_obj_get(obj, schema->name,
				strlen(schema->name), &child))) {
			evcpe_error(__func__, "failed to get attribute: %s", schema->name);
			goto finally;
		}
		if ((rc = evcpe_attr_to_xml(child, indent + 1, buffer))) {
			evcpe_error(__func__, "failed to marshal child attribute: %s",
					schema->name);
			goto finally;
		}
	}
	if ((rc = evcpe_xml_add_indent(buffer, EVCPE_XML_INDENT, indent))) {
		evcpe_error(__func__, "failed to append indentation");
		goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", attr->schema->name))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}

finally:
	return rc;
}

int evcpe_attr_to_xml(struct evcpe_attr *attr,
		unsigned int indent, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_obj_item *item;
	struct evcpe_access_list_item *entity;

	evcpe_debug(__func__, "marshalling evcpe_attr");

	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if ((rc = evcpe_attr_obj_to_xml(attr, attr->value.object, indent, buffer))) {
			evcpe_error(__func__, "failed to marshal evcpe_obj");
			goto finally;
		}
		break;
	case EVCPE_TYPE_MULTIPLE:
		if (TAILQ_EMPTY(&attr->value.multiple.list))
			break;
		TAILQ_FOREACH(item, &attr->value.multiple.list, entry) {
			if (!item->obj) continue;
			if ((rc = evcpe_attr_obj_to_xml(attr, item->obj, indent, buffer))) {
				evcpe_error(__func__, "failed to marshal evcpe_obj: %s",
						attr->schema->name);
				goto finally;
			}
		}
		break;
	default:
		if (!attr->value.simple.string &&
				attr->value.simple.notification == 0 &&
				TAILQ_EMPTY(&attr->value.simple.access_list.head))
			break;
		if ((rc = evcpe_xml_add_indent(buffer, EVCPE_XML_INDENT, indent))) {
			evcpe_error(__func__, "failed to append indentation");
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "<%s>", attr->schema->name))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		if (attr->value.simple.string) {
			if ((rc = evcpe_add_buffer(buffer, "<Value>%s</Value>",
					attr->value.simple.string))) {
				evcpe_error(__func__, "failed to append value: %s",
						attr->schema->name);
				goto finally;
			}
		}
		if (attr->value.simple.notification != 0) {
			if ((rc = evcpe_add_buffer(buffer,
					"<Notification>%d</Notification>",
					attr->value.simple.notification))) {
				evcpe_error(__func__, "failed to add notification: %s",
						attr->schema->name);
				goto finally;
			}
		}
		if (!TAILQ_EMPTY(&attr->value.simple.access_list.head)) {
			if ((rc = evcpe_add_buffer(buffer, "<AccessList>"))) {
				evcpe_error(__func__, "failed to append buffer");
				goto finally;
			}
			TAILQ_FOREACH(entity, &attr->value.simple.access_list.head, entry) {
				if ((rc = evcpe_add_buffer(buffer, "<string>%s</string>",
						entity->entity))) {
					evcpe_error(__func__, "failed to add access entity: %s",
							attr->schema->name);
					goto finally;
				}
			}
			if ((rc = evcpe_add_buffer(buffer, "</AccessList>"))) {
				evcpe_error(__func__, "failed to append buffer");
				goto finally;
			}
		}
		if ((rc = evcpe_add_buffer(buffer, "</%s>\n", attr->schema->name))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_to_xml_obj_param_name(struct evcpe_obj *obj,
		struct evbuffer *buffer)
{
	int rc;

	evcpe_debug(__func__, "marshaling object param name: %s",
			obj->class->name);

	if ((rc = evcpe_add_buffer(buffer, "<ParameterInfoStruct "
			"xsi:type=\""EVCPE_CWMP_XMLNS":ParameterInfoStruct\">"
			"<Name xsi:type=\"xsd:string\">")))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "%s", obj->path)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</Name>"
			"<Writable xsi:type=\"xsd:boolean\">true</Writable>"
			"</ParameterInfoStruct>")))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_to_xml_obj_param_names(struct evcpe_obj *obj,
		int next_level, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_attr *attr;
	struct evcpe_attr_schema *schema;

	evcpe_debug(__func__, "marshaling object to param names: %s",
			obj->class->name);

	TAILQ_FOREACH(schema, &obj->class->attrs, entry) {
		if (next_level && (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE))
			continue;
		if ((attr = evcpe_obj_find(obj, schema->name, strlen(schema->name))) &&
				(rc = evcpe_attr_to_xml_param_names(
						attr, next_level, buffer)))
			goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_to_xml_param_names(struct evcpe_attr *attr, int next_level,
		struct evbuffer *buffer)
{
	int rc;
	struct evcpe_obj_item *item;

	evcpe_debug(__func__, "marshaling attribute to param name: %s",
			attr->schema->name);

	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if (!next_level && (rc = evcpe_attr_to_xml_obj_param_name(
				attr->value.object, buffer)))
			goto finally;
		if ((rc = evcpe_attr_to_xml_obj_param_names(
				attr->value.object, next_level, buffer)))
			goto finally;
		break;
	case EVCPE_TYPE_MULTIPLE:
		TAILQ_FOREACH(item, &attr->value.multiple.list, entry) {
			if (!item->obj) continue;
			if ((rc = evcpe_attr_to_xml_obj_param_name(
					item->obj, buffer)))
				goto finally;
			if ((rc = evcpe_attr_to_xml_obj_param_names(
					item->obj, next_level, buffer)))
				goto finally;
		}
		break;
	default:
		if ((rc = evcpe_add_buffer(buffer, "<ParameterInfoStruct "
				"xsi:type=\""EVCPE_CWMP_XMLNS":ParameterInfoStruct\">"
				"<Name xsi:type=\"xsd:string\">")))
			goto finally;
		if ((rc = evcpe_add_buffer(buffer, "%s%s",
				attr->owner->path, attr->schema->name)))
			goto finally;
		if ((rc = evcpe_add_buffer(buffer, "</Name>"
				"<Writable xsi:type=\"xsd:boolean\">%s</Writable>"
				"</ParameterInfoStruct>",
				attr->schema->write == 'W' ? "true" : "false")))
			goto finally;
	}
	rc = 0;

finally:
	return rc;
}

#if 0
int evcpe_attr_count_xml_obj_param_names(struct evcpe_obj *obj,
		int next_level, unsigned int *count)
{
	int rc;
	struct evcpe_attr *attr;
	struct evcpe_attr_schema *schema;

	TAILQ_FOREACH(schema, &obj->class->attrs, entry) {
		if (next_level && (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE))
			continue;
		if ((attr = evcpe_obj_find(obj, schema->name, strlen(schema->name))) &&
				(rc = evcpe_attr_count_xml_param_names(
						attr, next_level, count)))
			goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_attr_count_xml_param_names(struct evcpe_attr *attr, int next_level,
		unsigned int *count)
{
	int rc;
	struct evcpe_obj_item *item;

//	*count = 0;
	switch (attr->schema->type) {
	case EVCPE_TYPE_OBJECT:
		if (!next_level)
			(*count) ++;
		if ((rc = evcpe_attr_count_xml_obj_param_names(
				attr->value.object, next_level, count)))
			goto finally;
		break;
	case EVCPE_TYPE_MULTIPLE:
		TAILQ_FOREACH(item, &attr->value.multiple.list, entry) {
			(*count) ++;
			if ((rc = evcpe_attr_count_xml_obj_param_names(
					item->obj, next_level, count)))
				goto finally;
		}
		break;
	default:
		(*count) ++;
		break;
	}
	rc = 0;

finally:
	return rc;
}
#endif
