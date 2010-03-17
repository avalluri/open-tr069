// $Id$

#include <stdlib.h>

#include "log.h"

#include "set_param_attrs.h"

struct evcpe_set_param_attrs *evcpe_set_param_attrs_new(void)
{
	struct evcpe_set_param_attrs *method;

	if (!(method = calloc(1, sizeof(struct evcpe_set_param_attrs)))) {
		evcpe_error(__func__, "failed to calloc evcpe_set_param_attrs");
		return NULL;
	}
	evcpe_set_param_attr_list_init(&method->parameter_list);
	return method;
}

void evcpe_set_param_attrs_free(struct evcpe_set_param_attrs *method)
{
	if (!method) return;

	evcpe_set_param_attr_list_clear(&method->parameter_list);
	free(method);
}
