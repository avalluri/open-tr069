// $Id$

#include "log.h"
#include "xml.h"

#include "fault_xml.h"

int evcpe_fault_to_xml(struct evcpe_fault *fault, struct evbuffer *buffer)
{
	int rc;

	evcpe_debug(__func__, "marshaling fault");

	if ((rc = evcpe_xml_add_int(buffer, "FaultCode", fault->code)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "FaultString", fault->string)))
		goto finally;

finally:
	return rc;

}
