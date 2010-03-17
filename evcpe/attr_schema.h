// $Id: attr_schema.h 248 2009-02-10 17:16:05Z cedric $

#ifndef EVCPE_ATTR_SCHEMA_H_
#define EVCPE_ATTR_SCHEMA_H_

#include <sys/queue.h>

#include "type.h"
#include "data.h"

struct evcpe_class;

struct evcpe_attr;

typedef int (*evcpe_attr_getter)(struct evcpe_attr *attr,
		const char **value, unsigned int *len);

typedef int (*evcpe_attr_setter)(struct evcpe_attr *attr,
		const char *value, unsigned int len);

struct evcpe_attr_schema {
	struct evcpe_class *owner;
	char *name;
	enum evcpe_type type;
	int extension;
	int inform;
	enum evcpe_notification notification;
	struct evcpe_class *class;
	char *number;
	struct evcpe_constraint constraint;
	char write;
	evcpe_attr_getter getter;
	evcpe_attr_setter setter;
	char *value;
	TAILQ_ENTRY(evcpe_attr_schema) entry;
};

void evcpe_attr_schema_free(struct evcpe_attr_schema *schema);

int evcpe_attr_schema_set_name(struct evcpe_attr_schema *schema,
		const char *name, unsigned len);

int evcpe_attr_schema_set_type(struct evcpe_attr_schema *schema,
		enum evcpe_type type);

int evcpe_attr_schema_set_default(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_number(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

#endif /* EVCPE_ATTR_SCHEMA_H_ */
