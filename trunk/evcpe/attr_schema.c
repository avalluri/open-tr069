// $Id: attr_schema.c 124 2009-01-11 11:50:11Z cedric $

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "util.h"
#include "class.h"

#include "attr_schema.h"

void evcpe_attr_schema_free(struct evcpe_attr_schema *schema)
{
	if (!schema) return;

	evcpe_trace(__func__, "destructing attribute schema: %s", schema->name);

	if (schema->class) evcpe_class_free(schema->class);
	if (schema->name) free(schema->name);
	if (schema->value) free(schema->value);
	if (schema->number) free(schema->number);
	if (schema->constraint.type == EVCPE_CONSTRAINT_ENUM)
		evcpe_constraint_enums_clear(&schema->constraint.value.enums);
	else if (schema->constraint.type == EVCPE_CONSTRAINT_ATTR)
		free(schema->constraint.value.attr);
	free(schema);
}

int evcpe_attr_schema_set_name(struct evcpe_attr_schema *schema,
		const char *name, unsigned len)
{
	evcpe_debug(__func__, "setting name: %.*s", len, name);
	return evcpe_strdup(name, len, &schema->name);
}

int evcpe_attr_schema_set_type(struct evcpe_attr_schema *schema,
		enum evcpe_type type)
{
	evcpe_trace(__func__, "setting type: %d", type);

	if ((type == EVCPE_TYPE_OBJECT || type == EVCPE_TYPE_MULTIPLE) &&
			!(schema->class = evcpe_class_new(schema->name))) {
		evcpe_error(__func__, "failed to create evcpe_class");
		return ENOMEM;
	}
	schema->type = type;
	return 0;
}

int evcpe_attr_schema_set_default(struct evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	evcpe_trace(__func__, "setting default: %.*s", len, value);
	return evcpe_strdup(value, len, &schema->value);
}

int evcpe_attr_schema_set_number(struct evcpe_attr_schema *schema,
		const char *value, unsigned len)
{
	evcpe_trace(__func__, "setting number of entries: %.*s", len, value);
	return evcpe_strdup(value, len, &schema->number);
}
