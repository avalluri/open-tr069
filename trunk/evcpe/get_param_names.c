// $Id$

#include <stdlib.h>

#include "log.h"

#include "get_param_names.h"

struct evcpe_get_param_names *evcpe_get_param_names_new(void)
{
	struct evcpe_get_param_names *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_names");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_names)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_names");
		return NULL;
	}
	return method;
}

void evcpe_get_param_names_free(struct evcpe_get_param_names *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_names");
	free(method);
}

struct evcpe_get_param_names_response *evcpe_get_param_names_response_new(void)
{
	struct evcpe_get_param_names_response *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_names_response");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_names_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_names");
		return NULL;
	}
	evcpe_param_info_list_init(&method->parameter_list);
	return method;
}

void evcpe_get_param_names_response_free(
		struct evcpe_get_param_names_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_names_response");
	evcpe_param_info_list_clear(&method->parameter_list);
	free(method);
}
