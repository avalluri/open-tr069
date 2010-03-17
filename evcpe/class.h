// $Id$

#ifndef EVCPE_CLASS_H_
#define EVCPE_CLASS_H_

#include "attr_schema.h"

TAILQ_HEAD(evcpe_attr_schemas, evcpe_attr_schema);

struct evcpe_class {
	const char *name;
	struct evcpe_attr_schemas attrs;
};

struct evcpe_class *evcpe_class_new(const char *name);

void evcpe_class_free(struct evcpe_class *class);

struct evcpe_attr_schema *evcpe_class_find(struct evcpe_class *class,
		const char *name, unsigned len);

int evcpe_class_add(struct evcpe_class *class,
		struct evcpe_attr_schema **schema);

int evcpe_class_add_attr(struct evcpe_class *class, const char *name,
		enum evcpe_type type, struct evcpe_attr_schema **schema);

#endif /* EVCPE_CLASS_H_ */
