// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "util.h"

#include "class.h"

struct evcpe_class *evcpe_class_new(const char *name)
{
	struct evcpe_class *class;

	evcpe_trace(__func__, "constructing evcpe_class: %s", name);

	if (!(class = calloc(1, sizeof(struct evcpe_class)))) {
		evcpe_error(__func__, "failed to calloc evcpe_class");
		return NULL;
	}
	class->name = name;
	TAILQ_INIT(&class->attrs);
	return class;
}

void evcpe_class_free(struct evcpe_class *class)
{
	struct evcpe_attr_schema *schema;

	if (!class) return;

	evcpe_trace(__func__, "destructing evcpe_class: %s", class->name);

	while((schema = TAILQ_FIRST(&class->attrs))) {
		TAILQ_REMOVE(&class->attrs, schema, entry);
		evcpe_attr_schema_free(schema);
	}
	free(class);
}

int evcpe_class_add(struct evcpe_class *class,
		struct evcpe_attr_schema **schema)
{
	int rc;

	evcpe_trace(__func__, "adding attribute to class: %s", class->name);

	if (!(*schema = calloc(1, sizeof(struct evcpe_attr_schema)))) {
		evcpe_error(__func__, "failed to calloc evcpe_attr_schema");
		rc = ENOMEM;
		goto finally;
	}
	(*schema)->owner = class;
	TAILQ_INSERT_TAIL(&class->attrs, *schema, entry);
	rc = 0;

finally:
	return rc;
}

struct evcpe_attr_schema *evcpe_class_find(struct evcpe_class *class,
		const char *name, unsigned len)
{
	struct evcpe_attr_schema *schema;

	TAILQ_FOREACH(schema, &class->attrs, entry) {
		if (!evcpe_strncmp(schema->name, name, len))
			return schema;
	}
	return NULL;
}
