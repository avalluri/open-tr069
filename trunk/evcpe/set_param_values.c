// $Id$

#include <stdlib.h>

#include "log.h"

#include "set_param_values.h"

struct evcpe_set_param_values *evcpe_set_param_values_new(void)
{
	struct evcpe_set_param_values *method;

	if (!(method = calloc(1, sizeof(struct evcpe_set_param_values)))) {
		evcpe_error(__func__, "failed to calloc evcpe_set_param_values");
		return NULL;
	}
	evcpe_set_param_value_list_init(&method->parameter_list);
	return method;
}

void evcpe_set_param_values_free(struct evcpe_set_param_values *method)
{
	if (!method) return;

	evcpe_set_param_value_list_clear(&method->parameter_list);
	free(method);
}

struct evcpe_set_param_values_response *evcpe_set_param_values_response_new(void)
{
	struct evcpe_set_param_values_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_set_param_values_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_set_param_values_response");
		return NULL;
	}
	return method;
}

void evcpe_set_param_values_response_free(struct evcpe_set_param_values_response *method)
{
	if (!method) return;

	free(method);
}
