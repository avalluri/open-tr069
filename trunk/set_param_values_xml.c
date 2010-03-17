// $Id$

#include "log.h"
#include "data_xml.h"

#include "set_param_values_xml.h"

int evcpe_set_param_values_response_to_xml(
		struct evcpe_set_param_values_response *method,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_debug(__func__, "marshaling evcpe_set_param_values_response");
	if ((rc = evcpe_xml_add_xsd_int(buffer, "Status",
			method->status)))
		goto finally;
finally:
	return rc;
}
