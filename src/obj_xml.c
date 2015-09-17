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

int evcpe_obj_to_xml(struct evcpe_obj *obj, struct evbuffer *buffer)
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

int evcpe_obj_from_xml(struct evcpe_obj *obj, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_obj_parser parser;
	struct evcpe_xml_element *elm;

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
	return rc;
}

int evcpe_obj_xml_elm_begin_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned len)
{
	int rc = 0;
	struct evcpe_obj_parser *parser = data;
	struct evcpe_xml_element *parent = NULL, *elm = NULL;
	struct evcpe_obj *parent_obj = NULL, *obj = NULL;
	struct evcpe_attr *attr = NULL;
	unsigned int index;

	TRACE("element begin: %.*s (namespace: %.*s)", len, name, nslen, ns);

	if (len >= sizeof(buffer)) {
		rc = EOVERFLOW;
		goto finally;
	}

	parent = evcpe_xml_stack_peek(&parser->stack);

	if (!(elm = calloc(1, sizeof(struct evcpe_xml_element)))) {
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

#if 0
	if (!evcpe_strncmp("Value", name, len)) {
	} else if (!evcpe_strncmp("Notification", name, len)) {
	} else if (!evcpe_strncmp("AccessList", name, len)) {
	} else if (!evcpe_strncmp("string", name, len)) {
		if (evcpe_strncmp("AccessList", parent->name, parent->len)) {
			ERROR("unexpected parent element: %.*s", parent->len, parent->name);
			rc = EPROTO;
			goto finally;
		}
	} else
#endif
	{
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
	}

	elm->data = obj;

finally:
	return rc;
}

int evcpe_obj_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc = 0;
	struct evcpe_obj_parser *parser = data;
	struct evcpe_xml_element *elm;

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) return -1;

	TRACE("element end: %.*s (namespace: %.*s)", len, name, nslen, ns);

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
	struct evcpe_obj_parser *parser = data;
	struct evcpe_xml_element *elm = NULL;
	struct evcpe_obj *obj = NULL;
	struct evcpe_attr *attr = NULL;
	struct evcpe_access_list_item *item = NULL;

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
	struct evcpe_xml_element *elm = NULL;
	struct evcpe_obj_parser *parser = data;

	TRACE("attribute %.*s=%.*s", name_len, name, value_len, value);
	if (!name_len || value_len) return -1;

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) {
		ERROR("Stack is empty");
		return -1;
	}
#if 0
	if (!evcpe_strncmp("notification", name, name_len)) {
		long n = 0;
		if ((rc = evcpe_obj_get((struct evcpe_obj *)elm->data, elm->name,
				elm->len, &attr))) {
			ERROR("failed to get attribute: %.*s", elm->len, elm->name);
			goto finally;
		}
		if ((rc = evcpe_atol(value, value_len, &n))) {
			ERROR("failed to convert to integer: %.*s", value_len, value);
			goto finally;
		}
		if ((rc = evcpe_attr_set_notification(attr, n))) {
			ERROR("failed to set notification: %ld", n);
			goto finally;
		}
	} else if (!evcpe_strncmp("string", elm->name, elm->len)) {
		if (!(parent = evcpe_xml_stack_up(parent))) {
			ERROR("no parent element");
			return -1;
		}
		if ((rc = evcpe_obj_get(obj, parent->name, parent->len, &attr))) {
			ERROR("failed to get attribute: %.*s",
					parent->len, parent->name);
			goto finally;
		}
		if ((attr->schema->type == EVCPE_TYPE_OBJECT ||
				attr->schema->type == EVCPE_TYPE_MULTIPLE)) {
			ERROR("not a simple attribute: %s", attr->schema->name);
			rc = -1;
			goto finally;
		}
		if ((rc = evcpe_access_list_add(
				&attr->value.simple.access_list, &item, text, len))) {
			ERROR("failed to add access list: %.*s", len, text);
			goto finally;
		}
	} else if (len > 0) {
		ERROR("unexpected element with text: %.*s", elm->len, elm->name);
		rc = EPROTO;
		goto finally;
	}
#endif

	return 0;
}
