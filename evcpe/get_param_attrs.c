// $Id$

#include <stdlib.h>

#include "log.h"

#include "get_param_attrs.h"

struct evcpe_get_param_attrs *evcpe_get_param_attrs_new(void)
{
	struct evcpe_get_param_attrs *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_attrs");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_attrs)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_attrs");
		return NULL;
	}
	evcpe_param_name_list_init(&method->parameter_names);
	return method;
}

void evcpe_get_param_attrs_free(struct evcpe_get_param_attrs *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_attrs");
	evcpe_param_name_list_clear(&method->parameter_names);
	free(method);
}

struct evcpe_get_param_attrs_response *evcpe_get_param_attrs_response_new(void)
{
	struct evcpe_get_param_attrs_response *method;
	evcpe_debug(__func__, "constructing evcpe_get_param_attrs_response");
	if (!(method = calloc(1, sizeof(struct evcpe_get_param_attrs_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_param_attrs");
		return NULL;
	}
	evcpe_param_attr_list_init(&method->parameter_list);
	return method;
}

void evcpe_get_param_attrs_response_free(
		struct evcpe_get_param_attrs_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_param_attrs_response");
	evcpe_param_attr_list_clear(&method->parameter_list);
	free(method);
}
