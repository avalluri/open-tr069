// $Id$

#include <stdlib.h>

#include "log.h"

#include "delete_object.h"

struct evcpe_delete_object *evcpe_delete_object_new(void)
{
	struct evcpe_delete_object *method;

	evcpe_debug(__func__, "constructing evcpe_delete_object");

	if (!(method = calloc(1, sizeof(struct evcpe_delete_object)))) {
		evcpe_error(__func__, "failed to calloc evcpe_add_obejct");
		return NULL;
	}
	return method;
}

void evcpe_delete_object_free(struct evcpe_delete_object *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_delete_object");
	free(method);
}
