// $Id$

#include "log.h"
#include "data_xml.h"

#include "get_rpc_methods_xml.h"

int evcpe_get_rpc_methods_response_to_xml(
		struct evcpe_get_rpc_methods_response *method,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_debug(__func__, "marshaling evcpe_get_rpc_methods_response");
	if ((rc = evcpe_method_list_to_xml(&method->method_list, "MethodList",
			buffer)))
		goto finally;
finally:
	return rc;
}
