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

	TRACE("element begin: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

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
			ERROR("<%.*s> should not be the root element",
					len, name);
			rc = EPROTO;
			goto finally;
		}
		parent_schema = parent->data;
		if (parent_schema->type != EVCPE_TYPE_OBJECT &&
				parent_schema->type != EVCPE_TYPE_MULTIPLE) {
			ERROR("sub-object is not applicable for "
					"type: %d", parent_schema->type);
			rc = EPROTO;
			goto finally;
		}
		if ((rc = evcpe_class_add(parent_schema->class, &schema))) {
			ERROR("failed to add attribute schema");
			goto finally;
		}
		if (extension)
			schema->extension = 1;
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
	int rc;
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
		if (TAILQ_EMPTY(&schema->class->attrs)) {
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
			if ((!schema->class || TAILQ_EMPTY(&schema->class->attrs))) {
				ERROR("missing sub-schema for object type");
				rc = EPROTO;
				goto finally;
			}
//		} else if (schema->type == EVCPE_TYPE_STRING) {
//			if (!schema->constraint.size) {
//				ERROR("missing constraint for string type");
//				rc = EPROTO;
//				goto finally;
//			}
		}
	}
	rc = 0;
	goto finally;

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
	int rc;
	long val;
	struct evcpe_class_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_attr_schema *schema, *find;
	const char *start, *end;
	char symbol[256];
	const char *error;

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) return -1;

	TRACE("attribute: %.*s => %.*s (namespace: %.*s)",
			name_len, name, value_len, value, nslen, ns);

	schema = elm->data;
	if (!evcpe_strncmp("name", name, name_len)) {
		if (evcpe_strncmp("schema", elm->name, elm->len) &&
				evcpe_strncmp("extension", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		if ((rc = evcpe_attr_schema_set_name(schema, value, value_len))) {
			ERROR("failed to set attribute schema name: %.*s",
					value_len, value);
			goto finally;
		}
	} else if (!evcpe_strncmp("type", name, name_len)) {
		if (evcpe_strncmp("schema", elm->name, elm->len) &&
				evcpe_strncmp("extension", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		if (!evcpe_strncmp("object", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_OBJECT)))
				goto finally;
		} else if (!evcpe_strncmp("multipleObject", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_MULTIPLE)))
				goto finally;
		} else if (!evcpe_strncmp("string", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_STRING)))
				goto finally;
		} else if (!evcpe_strncmp("int", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_INT)))
				goto finally;
		} else if (!evcpe_strncmp("unsignedInt", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_UNSIGNED_INT)))
				goto finally;
		} else if (!evcpe_strncmp("boolean", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_BOOLEAN)))
				goto finally;
		} else if (!evcpe_strncmp("dateTime", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_DATETIME)))
				goto finally;
		} else if (!evcpe_strncmp("base64", value, value_len)) {
			if ((rc = evcpe_attr_schema_set_type(schema,
					EVCPE_TYPE_BASE64)))
				goto finally;
		} else {
			ERROR("unexpected type: %.*s",
					value_len, value);
			rc = EPROTO;
			goto finally;
		}
	} else if (!evcpe_strncmp("constraint", name, name_len)) {
		switch(schema->type) {
		case EVCPE_TYPE_STRING:
		case EVCPE_TYPE_BASE64:
			if (!evcpe_atol(value, value_len, &val)) {
				if (val < 0) {
					ERROR("size constraint should not be "
							"negative: %ld", val);
					rc = EPROTO;
					goto finally;
				}
				schema->constraint.type = EVCPE_CONSTRAINT_SIZE;
				schema->constraint.value.size = val;
				schema->constraint.pattern =
            		"^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$";
			} else if (schema->type == EVCPE_TYPE_STRING) {
				schema->constraint.type = EVCPE_CONSTRAINT_ENUM;
				TAILQ_INIT(&schema->constraint.value.enums);
				for (start = end = value; end < value + value_len; end ++) {
					if (*end == '|') {
						if ((rc = evcpe_constraint_enums_add(
								&schema->constraint.value.enums,
								start, end - start))) {
							ERROR("failed to add enum constraint");
							goto finally;
						}
						start = end + 1;
					}
				}
				if ((rc = evcpe_constraint_enums_add(
						&schema->constraint.value.enums, start, end - start))) {
					ERROR("failed to add enum constraint");
					goto finally;
				}
			} else {
				ERROR("invalid constraint: %.*s",
						value_len, value);
				rc = EPROTO;
				goto finally;
			}
			break;
		case EVCPE_TYPE_INT:
		case EVCPE_TYPE_UNSIGNED_INT:
			end = NULL;
			for (start = value; start < value + value_len; start ++) {
				if (*start == ':') {
					end = start;
					break;
				}
			}
			if (!end) {
				ERROR("constraint of int/unsignedInt should be "
						"a colon separated integer pair");
				rc = EPROTO;
				goto finally;
			}
			if (end == value) {
				if ((rc = evcpe_constraint_set_max(&schema->constraint,
						value + 1, value_len - 1))) {
					ERROR("failed to set max constraint");
					goto finally;
				}
			} else if (end == value + value_len - 1) {
				if ((rc = evcpe_constraint_set_min(&schema->constraint,
						value, end - value))) {
					ERROR("failed to set min constraint");
					goto finally;
				}
			} else {
				if ((rc = evcpe_constraint_set_range(&schema->constraint,
						value, end - value,
						end + 1, value_len - (end - value + 1)))) {
					ERROR("failed to set range constraint");
					goto finally;
				}
			}
			break;
		case EVCPE_TYPE_MULTIPLE:
			if (!(find = evcpe_class_find(schema->owner, value, value_len))) {
				ERROR("constraint attribute doesn't exist: "
						"%.*s", value_len, value);
				rc = EPROTO;
				goto finally;
			}
			if (find->type != EVCPE_TYPE_UNSIGNED_INT) {
				ERROR("constraint attribute is not "
						"unsigned int: %.*s", value_len, value);
				rc = EPROTO;
				goto finally;
			}
			if ((rc = evcpe_constraint_set_attr(&schema->constraint,
					value, value_len))) {
				ERROR("failed to set attribute constraint");
				goto finally;
			}
			break;
		case EVCPE_TYPE_UNKNOWN:
		case EVCPE_TYPE_OBJECT:
		case EVCPE_TYPE_BOOLEAN:
		case EVCPE_TYPE_DATETIME:
		default:
			ERROR("constraint is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
  } else if (!evcpe_strncmp("pattern", name, name_len)) {
    // TODO: Add support for regex
    if ((rc = evcpe_constraint_set_pattern(&schema->constraint, value, value_len))) {
      ERROR("failed to set constraint pattern: %*.s", value_len, value);
      rc = EPROTO;
      goto finally;
    }
	} else if (!evcpe_strncmp("default", name, name_len)) {
		if (schema->type == EVCPE_TYPE_DATETIME) {
			if (evcpe_strncmp("auto", value, value_len)) {
				ERROR("only \"auto\" is accepted for default "
						"dateTime value");
				goto finally;
			}
		} else if ((rc = evcpe_type_validate(schema->type, value, value_len,
				&schema->constraint))) {
			ERROR("invalid default value: %.*s",
					value_len, value);
			goto finally;
		}
		if ((rc = evcpe_attr_schema_set_default(schema, value, value_len))) {
			ERROR("failed to set attr schema default: %.*s",
					value_len, value);
			goto finally;
		}
	} else if (!evcpe_strncmp("number", name, name_len)) {
		if (schema->type != EVCPE_TYPE_MULTIPLE) {
			ERROR("number of entries is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (!(find = evcpe_class_find(schema->owner, value, value_len))) {
			ERROR("number of entries attribute doesn't exist: "
					"%.*s", value_len, value);
			rc = EPROTO;
			goto finally;
		}
		if (find->type != EVCPE_TYPE_UNSIGNED_INT) {
			ERROR("number of entries attribute is not "
					"unsigned int: %.*s", value_len, value);
			rc = EPROTO;
			goto finally;
		}
		if ((rc = evcpe_attr_schema_set_number(schema, value, value_len))) {
			ERROR("failed to set attr schema default: %.*s",
					value_len, value);
			goto finally;
		}
	} else if (!evcpe_strncmp("write", name, name_len)) {
		if (schema->type == EVCPE_TYPE_OBJECT) {
			ERROR("'write' is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (evcpe_strncmp("schema", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		if (value_len != 1) {
			ERROR("write attribute should be a character");
			rc = EPROTO;
			goto finally;
		}
		if (value[0] != 'W') {
			ERROR("unexpected write attribute: %.*s", 1, value);
			rc = EPROTO;
			goto finally;
		}
		schema->write = value[0];
	} else if (!evcpe_strncmp("inform", name, name_len)) {
		if (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("'inform' is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (evcpe_strncmp("schema", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		if (!evcpe_strncmp("true", value, value_len)) {
			schema->inform = 1;
		} else if (evcpe_strncmp("false", value, value_len)) {
			ERROR("'inform' value must be (true|false)': %.*s",
					value_len, value);
			rc = EPROTO;
			goto finally;
		}
	} else if (!evcpe_strncmp("notification", name, name_len)) {
		if (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("'notification' is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (evcpe_strncmp("schema", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		if (!evcpe_strncmp("passive", value, value_len)) {
			schema->notification = EVCPE_NOTIFICATION_PASSIVE;
		} else if (!evcpe_strncmp("active", value, value_len)) {
				schema->notification = EVCPE_NOTIFICATION_ACTIVE;
		} else if (evcpe_strncmp("false", value, value_len)) {
			ERROR("'notification' value must be (normal|passive|active): %.*s",
					value_len, value);
			rc = EPROTO;
			goto finally;
		}
	} else if (!evcpe_strncmp("getter", name, name_len)) {
		if (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("'notification' is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (evcpe_strncmp("schema", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		snprintf(symbol, sizeof(symbol), "%.*s", value_len, value);
		if (!(schema->getter = dlsym(parser->dynlib, symbol)) &&
				(error = dlerror())) {
			ERROR("failed to load symbol: %s", error);
			rc = -1;
			goto finally;
		}
	} else if (!evcpe_strncmp("setter", name, name_len)) {
		if (schema->type == EVCPE_TYPE_OBJECT ||
				schema->type == EVCPE_TYPE_MULTIPLE) {
			ERROR("'notification' is not applicable to "
					"type: %d", schema->type);
			rc = EPROTO;
			goto finally;
		}
		if (evcpe_strncmp("schema", elm->name, elm->len)) {
			ERROR("unexpected element: %.*s",
					elm->len, elm->name);
			rc = EPROTO;
			goto finally;
		}
		snprintf(symbol, sizeof(symbol), "%.*s", value_len, value);
		if (!(schema->setter = dlsym(parser->dynlib, symbol)) &&
				(error = dlerror())) {
			ERROR("failed to load symbol: %s", error);
			rc = -1;
			goto finally;
		}
	} else if(!evcpe_strncmp("description", name, name_len)) {
        	// ignore descriotion
	}else if(!evcpe_strncmp("status", name, name_len)){
    if (!evcpe_strncmp("deprecated", elm->name, elm->len)) {
      WARN("Deprecated parameter: %.*s", elm->len, elm->name);
		}
	} else {
		ERROR("unexpected attribute: %.*s",
				name_len, name);
		rc = EPROTO;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}
