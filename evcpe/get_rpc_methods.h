// $Id$

#ifndef EVCPE_GET_RPC_METHODS_H_
#define EVCPE_GET_RPC_METHODS_H_

#include "data.h"

struct evcpe_get_rpc_methods {
};

struct evcpe_get_rpc_methods *evcpe_get_rpc_methods_new(void);

void evcpe_get_rpc_methods_free(struct evcpe_get_rpc_methods *method);

struct evcpe_get_rpc_methods_response {
	struct evcpe_method_list method_list;
};

struct evcpe_get_rpc_methods_response *evcpe_get_rpc_methods_response_new(void);

void evcpe_get_rpc_methods_response_free(
		struct evcpe_get_rpc_methods_response *resp);

#endif /* EVCPE_GET_RPC_METHODS_H_ */
