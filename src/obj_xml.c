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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "util.h"
#include "attr_xml.h"

#include "obj_xml.h"

static char buffer[257];

static int evcpe_obj_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_obj_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_obj_xml_data_cb(void *data,
		const char *text, unsigned len);
static int evcpe_obj_xml_attr_cb(void *data,
		const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len);

int evcpe_obj_to_xml(evcpe_obj *obj, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<?xml version=\"1.0\"?>\n"))) {
		ERROR("failed to append buffer");
		goto finally;
	}
	if ((rc = evcpe_attr_to_xml(RB_ROOT(&obj->attrs), 0, buffer))) {
		ERROR("failed to marshal root attribute");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_obj_from_xml(evcpe_obj *obj, struct evbuffer *buffer)
{
	int rc;
	evcpe_obj_parser parser;
	evcpe_xml_element *elm;

	DEBUG("unmarshalling object from XML");

	parser.root = obj;
	parser.xml.data = &parser;
	parser.xml.xmlstart = (const char *)evbuffer_pullup(buffer, -1);
	parser.xml.xmlsize = evbuffer_get_length(buffer);
	parser.xml.starteltfunc = evcpe_obj_xml_elm_begin_cb;
	parser.xml.endeltfunc = evcpe_obj_xml_elm_end_cb;
	parser.xml.datafunc = evcpe_obj_xml_data_cb;
	parser.xml.attfunc = evcpe_obj_xml_attr_cb;
	SLIST_INIT(&parser.stack);
	if ((rc = parsexml(&parser.xml))) {
		ERROR("failed to parse XML: %d", rc);
	}
	while((elm = evcpe_xml_stack_pop(&parser.stack))) {
		ERROR("pending stack: %.*s", elm->len, elm->name);
		free(elm);
	}

  //evcpe_obj_dump(obj);
	return rc;
}

int evcpe_obj_xml_elm_begin_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned len)
{
	int rc = 0;
	evcpe_obj_parser *parser = data;
	evcpe_xml_element *parent = NULL, *elm = NULL;
	evcpe_obj *parent_obj = NULL, *obj = NULL;
	evcpe_attr *attr = NULL;
	unsigned int index;

	TRACE("element begin: %.*s (namespace: %.*s)", len, name, nslen, ns);

	if (len >= sizeof(buffer)) {
		rc = EOVERFLOW;
		goto finally;
	}

	parent = evcpe_xml_stack_peek(&parser->stack);

	if (!(elm = calloc(1, sizeof(evcpe_xml_element)))) {
		ERROR("failed to calloc evcpe_soap_element");
		return ENOMEM;
	}
	elm->ns = ns;
	elm->nslen = nslen;
	elm->name = name;
	elm->len = len;
	evcpe_xml_stack_put(&parser->stack, elm);

	if (!parent) {
		obj = parser->root;
	} else if (!(obj = parent->data)) {
		ERROR("no object in parent element: %.*s", parent->len, parent->name);
		rc = -1;
		goto finally;
	}

	if ((rc = evcpe_obj_get(obj, name, len, &attr))) {
		ERROR("failed to find attribute in class: %.*s", len, name);
		goto finally;
	}
	if (attr->schema->type == EVCPE_TYPE_OBJECT)
		obj = attr->value.object;
	else if (attr->schema->type == EVCPE_TYPE_MULTIPLE) {
		if ((rc = evcpe_attr_add_obj(attr, &obj, &index))) {
			ERROR("failed to add object: %.*s", len, name);
			goto finally;
		}
	}

	elm->data = obj;

finally:
	return rc;
}

int evcpe_obj_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc = 0;
	evcpe_obj_parser *parser = data;
	evcpe_xml_element *elm;

	TRACE("element end: %.*s (namespace: %.*s)", len, name, nslen, ns);

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) {
		ERROR("No element found in the stack");
		return -1;
	}

	if ((nslen && evcpe_strcmp(elm->ns, elm->nslen, ns, nslen)) ||
			evcpe_strcmp(elm->name, elm->len, name, len)) {
		ERROR("element doesn't match start: %.*s:%.*s", nslen, ns, len, name);
		rc = EPROTO;
		goto finally;
	}

finally:
	free(elm);
	return rc;
}

int evcpe_obj_xml_data_cb(void *data, const char *text, unsigned len)
{
	int rc = 0;
	evcpe_obj_parser *parser = data;
	evcpe_xml_element *elm = NULL;
	evcpe_obj *obj = NULL;
	evcpe_attr *attr = NULL;
	struct evcpe_access *item = NULL;

	if (!len) return 0;

	TRACE("text: %.*s",	len, text);

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) {
		ERROR("Stack is empty");
		return -1;
	}

	obj = elm->data;
	if (!obj) {
		ERROR("elemnt '%.*s' has NULL object", elm->len, elm->name);
		return -1;
	}

	if ((rc = evcpe_obj_get(obj, elm->name, elm->len, &attr))) {
		ERROR("failed to get attribute: %.*s in %s", elm->len, elm->name,
				obj->path);
		goto finally;
	}

	if ((rc = evcpe_attr_set(attr, text, len))) {
		ERROR("failed to set simple attr: %.*s", elm->len, elm->name);
		goto finally;
	}

finally:
	return rc;
}

int evcpe_obj_xml_attr_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc = 0;
	evcpe_xml_element *elm = NULL;
	evcpe_obj *obj = NULL;
	evcpe_obj_parser *parser = data;

	TRACE("attribute %.*s=%.*s", name_len, name, value_len, value);
	if (!name_len || !value_len) {
		ERROR("Empty name(%.*s) or value(%.*s)", name_len, name, value_len,
				value);
		return -1;
	}

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) {
		ERROR("Stack is empty");
		return -1;
	}

	obj = elm->data;
	if (!evcpe_strncmp("notification", name, name_len)) {
		long n = 0;
		evcpe_attr *attr = NULL;

		if ((rc = evcpe_atol(value, value_len, &n))) {
			ERROR("failed to convert to integer: %.*s", value_len, value);
			return rc;
		}
		if ((rc = evcpe_obj_get(obj, elm->name, elm->len, &attr))) {
			ERROR("failed to get attribute: %.*s", elm->len, elm->name);
			return rc;
		}
		if ((rc = evcpe_attr_set_notification(attr, n))) {
			ERROR("failed to set notification: %ld", n);
			return rc;
		}
	} else if (!evcpe_strncmp("access-list", name, name_len)) {
		evcpe_attr *attr = NULL;

		if ((rc = evcpe_obj_get(obj, elm->name, elm->len, &attr))) {
			ERROR("failed to get attribute: %.*s",elm->len, elm->name);
			return rc;
		}

		return evcpe_attr_set_access_list_from_str(attr, value, value_len);
	} else
	{
		ERROR("unexpected attribute %.*s with value :%.*s",
				name_len, name, value_len, value);
		return EPROTO;
	}

	return 0;
}
