// $Id$

#include "log.h"
#include "data_xml.h"

#include "add_object_xml.h"

int evcpe_add_object_response_to_xml(
		struct evcpe_add_object_response *method,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_debug(__func__, "marshaling evcpe_add_object_response");
	if ((rc = evcpe_xml_add_xsd_unsigned_int(buffer, "InstanceNumber",
			method->instance_number)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_int(buffer, "Status",
			method->status)))
		goto finally;
finally:
	return rc;
}
