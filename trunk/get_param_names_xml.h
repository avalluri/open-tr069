// $Id$

#ifndef EVCPE_GET_PARAM_NAMES_XML_H_
#define EVCPE_GET_PARAM_NAMES_XML_H_

#include <event.h>

#include "get_param_names.h"

int evcpe_get_param_names_response_to_xml(
		struct evcpe_get_param_names_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_GET_PARAM_NAMES_XML_H_ */
