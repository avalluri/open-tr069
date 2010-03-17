// $Id$

#ifndef EVCPE_SET_PARAM_VALUES_XML_H_
#define EVCPE_SET_PARAM_VALUES_XML_H_

#include <event.h>

#include "set_param_values.h"

int evcpe_set_param_values_response_to_xml(
		struct evcpe_set_param_values_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_SET_PARAM_VALUES_XML_H_ */
