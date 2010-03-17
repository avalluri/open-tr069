// $Id$

#ifndef EVCPE_OBJ_H_
#define EVCPE_OBJ_H_

#include <sys/types.h>

#include "attr.h"

struct evcpe_obj {
	char *path;
	unsigned int pathlen;
	struct evcpe_class *class;
	struct evcpe_attr *owner;
	unsigned int index;
	struct evcpe_attrs attrs;
};

struct evcpe_obj *evcpe_obj_new(struct evcpe_class *class,
		struct evcpe_attr *owner);

void evcpe_obj_free(struct evcpe_obj *obj);

int evcpe_obj_init(struct evcpe_obj *obj);

struct evcpe_attr *evcpe_obj_find(struct evcpe_obj *obj,
		const char *name, unsigned len);

int evcpe_obj_get(struct evcpe_obj *obj,
		const char *name, unsigned len, struct evcpe_attr **attr);

#endif /* EVCPE_OBJ_H_ */
