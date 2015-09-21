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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "log.h"
#include "util.h"

#include "class_xml.h"

#define EVCPE_CLASS_XML_MAX_ATTRIBUTES 10

static int evcpe_class_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_class_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_class_xml_data_cb(void *data,
		const char *text, unsigned len);
static int evcpe_class_xml_attr_cb(void *data,
		const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len);

int evcpe_class_from_xml(struct evcpe_class *class, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_class_parser parser;
	struct evcpe_attr_schema root;
	struct evcpe_xml_element *elm;

	DEBUG("unmarshalling class from XML");

	root.name = NULL;
	root.class = class;
	root.type = EVCPE_TYPE_OBJECT;
	if (!(parser.dynlib = dlopen(NULL, RTLD_LAZY|RTLD_GLOBAL))) {
		ERROR("failed to open current process as dynamic lib: %s", dlerror());
		return -1;
	}
	parser.root = &root;
	parser.xml.data = &parser;
	parser.xml.xmlstart = (const char *)evbuffer_pullup(buffer, -1);
	parser.xml.xmlsize = evbuffer_get_length(buffer);
	parser.xml.starteltfunc = evcpe_class_xml_elm_begin_cb;
	parser.xml.endeltfunc = evcpe_class_xml_elm_end_cb;
	parser.xml.datafunc = evcpe_class_xml_data_cb;
	parser.xml.attfunc = evcpe_class_xml_attr_cb;
	SLIST_INIT(&parser.stack);
	if ((rc = parsexml(&parser.xml))) {
		ERROR("failed to parse XML: %d", rc);
	}
	while((elm = evcpe_xml_stack_pop(&parser.stack))) {
		ERROR("pending stack: %.*s", elm->len, elm->name);
		free(elm);
	}
	if (root.name) free(root.name);
	if (parser.dynlib) dlclose(parser.dynlib);
	return rc;
}

int evcpe_class_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc, extension;
	struct evcpe_class_parser *parser = data;
	struct evcpe_xml_element *parent, *elm;
	struct evcpe_attr_schema *schema, *parent_schema;

	TRACE("element begin: %.*s (namespace: %.*s)", len, name, nslen, ns);

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

	if (!evcpe_strncmp("model", name, len)) {
		if (parent) {
			ERROR("<model> should be the root element");
			rc = EPROTO;
			goto finally;
		}
		elm->data = parser->root;
	} else if ((extension = !evcpe_strncmp("extension", name, len)) ||
			!evcpe_strncmp("schema", name, len)) {
		if (!parent) {
			ERROR("<%.*s> should not be the root element", len, name);
			rc = EPROTO;
			goto finally;
		}
		parent_schema = parent->data;
		if (parent_schema->type != EVCPE_TYPE_OBJECT &&
			parent_schema->type != EVCPE_TYPE_MULTIPLE) {
			ERROR("sub-object is not applicable for type: %d",
					parent_schema->type);
			rc = EPROTO;
			goto finally;
		}
		if ((rc = evcpe_class_add_new_schema(parent_schema->class, &schema))) {
			ERROR("failed to add attribute schema");
			goto finally;
		}
		evcpe_attr_schema_set_extension(schema, extension);
		elm->data = schema;
	} else {
		ERROR("unexpected element: %.*s", len, name);
		rc = EPROTO;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_class_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc = 0;
	struct evcpe_class_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_attr_schema *schema;

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) return -1;

	TRACE("element end: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

	if ((nslen && evcpe_strcmp(elm->ns, elm->nslen, ns, nslen)) ||
			evcpe_strcmp(elm->name, elm->len, name, len)) {
		ERROR("element doesn't match start: %.*s:%.*s",
				nslen, ns, len, name);
		rc = EPROTO;
		goto finally;
	}

	schema = elm->data;
	if (!evcpe_strncmp("model", name, len)) {
		if (tqueue_empty(schema->class->attrs)) {
			ERROR("empty model definition");
			rc = EPROTO;
			goto finally;
		}
	} else if (!evcpe_strncmp("schema", name, len)) {
		if (!schema->name) {
			ERROR("missing schema name");
			rc = EPROTO;
			goto finally;
		}
		if (schema->type == EVCPE_TYPE_UNKNOWN) {
			ERROR("missing schema type");
			rc = EPROTO;
			goto finally;
		} else if ((schema->type == EVCPE_TYPE_OBJECT ||
				    schema->type == EVCPE_TYPE_MULTIPLE)) {
			if ((!schema->class || tqueue_empty(schema->class->attrs))) {
				ERROR("missing sub-schema for object type");
				rc = EPROTO;
				goto finally;
			}
		}
	}

finally:
	free(elm);
	return rc;
}

int evcpe_class_xml_data_cb(void *data, const char *text, unsigned len)
{
	if (len) {
		ERROR("XML text is not expected");
		return EPROTO;
	}
	return 0;
}

int evcpe_class_xml_attr_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc = 0;
	long val = 0;
	struct evcpe_class_parser *parser = data;
	struct evcpe_xml_element *elm = NULL;
	struct evcpe_attr_schema *schema = NULL;
	const char *error = NULL;
	int getter = 0;

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) return -1;

	TRACE("attribute: %.*s => %.*s (namespace: %.*s)",
			name_len, name, value_len, value, nslen, ns);

	schema = elm->data;
	if (!evcpe_strncmp("name", name, name_len)) {
		return evcpe_attr_schema_set_name(schema, value, value_len);
	}

	if (!evcpe_strncmp("type", name, name_len)) {
		enum evcpe_type type = evcpe_type_from_str(value, value_len);
		if (type == EVCPE_TYPE_UNKNOWN) {
			ERROR("unexpected type: %.*s", value_len, value);
			return EPROTO;
		}

		return evcpe_attr_schema_set_type(schema, type);
	}

	if (!evcpe_strncmp("constraint", name, name_len)) {
		return evcpe_attr_schema_set_constraint(schema, value, value_len);
	}

	if (!evcpe_strncmp("pattern", name, name_len)) {
		// TODO: Add support for regex
		return evcpe_attr_schema_set_pattern(schema, value, value_len);
	}

	if (!evcpe_strncmp("default", name, name_len)) {
		return evcpe_attr_schema_set_default(schema, value, value_len);
	}

	if (!evcpe_strncmp("number", name, name_len)) {
		return evcpe_attr_schema_set_number(schema, value, value_len);
	}

	if(!evcpe_strncmp("description", name, name_len)) {
		// ignore descriotion
		return 0;
	}

	if(!evcpe_strncmp("status", name, name_len)) {
		if (!evcpe_strncmp("deprecated", elm->name, elm->len)) {
			WARN("Deprecated parameter: %.*s", elm->len, elm->name);
		}
		return 0;
	}

	if (!schema->extension) {
		if (!evcpe_strncmp("write", name, name_len)) {
			return evcpe_attr_schema_set_write(schema, 1);
		}

		if (!evcpe_strncmp("inform", name, name_len)) {
			if (schema->type == EVCPE_TYPE_OBJECT ||
					schema->type == EVCPE_TYPE_MULTIPLE) {
				ERROR("'inform' is not applicable to type: %d", schema->type);
				return EPROTO;
			}

			schema->inform = 1;
			/* TODO: find better way to add this schema to inform schema list */
			tqueue_insert(parser->root->class->inform_attrs, schema);

			return 0;
		}

		if (!evcpe_strncmp("notification", name, name_len)) {
			return evcpe_attr_schema_set_notification(schema, value, value_len);
		}

		if ((getter = !evcpe_strncmp("getter", name, name_len)) ||
			!evcpe_strncmp("setter", name, name_len)) {
			char symbol_name[256];
			void *symbol = NULL;
			if (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE) {
				ERROR("'notification' is not applicable to type: %d",
						schema->type);
				return EPROTO;
			}

			snprintf(symbol_name, sizeof(symbol_name), "%.*s", value_len, value);
			if (!(symbol = dlsym(parser->dynlib, symbol_name)) &&
					(error = dlerror())) {
				ERROR("failed to load symbol: %s", error);
				return -1;
			}

			if (getter) schema->getter = symbol;
			else schema->setter = symbol;

			return 0;
		}
	}

	ERROR("unexpected attribute: %.*s", name_len, name);
	return EPROTO;
}
