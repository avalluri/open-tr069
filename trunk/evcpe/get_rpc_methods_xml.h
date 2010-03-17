// $Id$

#ifndef EVCPE_GET_RPC_METHODS_XML_H_
#define EVCPE_GET_RPC_METHODS_XML_H_

#include <event.h>

#include "get_rpc_methods.h"

int evcpe_get_rpc_methods_response_to_xml(
		struct evcpe_get_rpc_methods_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_GET_RPC_METHODS_XML_H_ */
