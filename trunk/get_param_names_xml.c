// $Id$

#include "log.h"
#include "data_xml.h"

#include "get_param_names_xml.h"

int evcpe_get_param_names_response_to_xml(
		struct evcpe_get_param_names_response *method,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_debug(__func__, "marshaling evcpe_get_param_names_response");
	if ((rc = evcpe_param_info_list_to_xml(&method->parameter_list,
			"ParameterList", buffer)))
		goto finally;
finally:
	return rc;
}
