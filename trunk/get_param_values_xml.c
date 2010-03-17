// $Id$

#include "log.h"
#include "data_xml.h"

#include "get_param_values_xml.h"

int evcpe_get_param_values_response_to_xml(
		struct evcpe_get_param_values_response *method,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_debug(__func__, "marshaling evcpe_get_param_values_response");
	if ((rc = evcpe_param_value_list_to_xml(&method->parameter_list,
			"ParameterList", buffer)))
		goto finally;
finally:
	return rc;
}
