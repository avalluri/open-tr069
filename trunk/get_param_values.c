// $Id$

#include <stdlib.h>

#include "log.h"

#include "get_param_values.h"

struct evcpe_get_param_values *evcpe_get_param_values_new(void)
{
	struct evcpe_get_param_values *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_values");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_values)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_values");
		return NULL;
	}
	evcpe_param_name_list_init(&method->parameter_names);
	return method;
}

void evcpe_get_param_values_free(struct evcpe_get_param_values *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_values");
	evcpe_param_name_list_clear(&method->parameter_names);
	free(method);
}

struct evcpe_get_param_values_response *evcpe_get_param_values_response_new(void)
{
	struct evcpe_get_param_values_response *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_values_response");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_values_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_values");
		return NULL;
	}
	evcpe_param_value_list_init(&method->parameter_list);
	return method;
}

void evcpe_get_param_values_response_free(
		struct evcpe_get_param_values_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_values_response");
	evcpe_param_value_list_clear(&method->parameter_list);
	free(method);
}
