// $Id$

#include <stdlib.h>

#include "log.h"

#include "add_object.h"

struct evcpe_add_object *evcpe_add_object_new(void)
{
	struct evcpe_add_object *method;

	evcpe_debug(__func__, "constructing evcpe_add_object");

	if (!(method = calloc(1, sizeof(struct evcpe_add_object)))) {
		evcpe_error(__func__, "failed to calloc evcpe_add_obejct");
		return NULL;
	}
	return method;
}

void evcpe_add_object_free(struct evcpe_add_object *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_add_object");
	free(method);
}

struct evcpe_add_object_response *evcpe_add_object_response_new(void)
{
	struct evcpe_add_object_response *method;

	evcpe_debug(__func__, "constructing evcpe_add_object_response");

	if (!(method = calloc(1, sizeof(struct evcpe_add_object_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_add_obejct_response");
		return NULL;
	}
	return method;
}

void evcpe_add_object_response_free(struct evcpe_add_object_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_add_object_response");
	free(method);
}
