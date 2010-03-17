// $Id$

#include "log.h"
#include "data_xml.h"

#include "inform_xml.h"

int evcpe_inform_to_xml(struct evcpe_inform *method, struct evbuffer *buffer)
{
	int rc;

	evcpe_debug(__func__, "marshaling inform");

	if ((rc = evcpe_device_id_to_xml(&method->device_id, "DeviceId", buffer)))
		goto finally;
	if ((rc = evcpe_event_list_to_xml(&method->event, "Event", buffer)))
		goto finally;
	if ((rc = evcpe_xml_add_int(buffer,
			"MaxEnvelopes", method->max_envelopes)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer,
			"CurrentTime", method->current_time)))
		goto finally;
	if ((rc = evcpe_xml_add_int(buffer,
			"RetryCount", method->retry_count)))
		goto finally;
	if ((rc = evcpe_param_value_list_to_xml(&method->parameter_list,
			"ParameterList", buffer)))
		goto finally;

finally:
	return rc;
}
