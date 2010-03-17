// $Id$

#include <stdlib.h>

#include "log.h"

#include "get_rpc_methods.h"

struct evcpe_get_rpc_methods *evcpe_get_rpc_methods_new(void)
{
	struct evcpe_get_rpc_methods *method;

	evcpe_debug(__func__, "constructing evcpe_get_rpc_methods");

	if (!(method = calloc(1, sizeof(struct evcpe_get_rpc_methods)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_rpc_methods");
		return NULL;
	}
	return method;
}

void evcpe_get_rpc_methods_free(struct evcpe_get_rpc_methods *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_rpc_methods");
	free(method);
}

struct evcpe_get_rpc_methods_response *evcpe_get_rpc_methods_response_new(void)
{
	struct evcpe_get_rpc_methods_response *method;
	evcpe_debug(__func__, "constructing evcpe_get_rpc_methods_response");
	if (!(method = calloc(1, sizeof(struct evcpe_get_rpc_methods_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_rpc_methods_response");
		return NULL;
	}
	evcpe_method_list_init(&method->method_list);
	return method;
}

void evcpe_get_rpc_methods_response_free(
		struct evcpe_get_rpc_methods_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_rpc_methods_response");
	evcpe_method_list_clear(&method->method_list);
	free(method);
}
