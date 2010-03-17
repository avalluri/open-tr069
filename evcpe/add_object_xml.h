// $Id$

#ifndef EVCPE_ADD_OBJECT_XML_H_
#define EVCPE_ADD_OBJECT_XML_H_

#include <event.h>

#include "add_object.h"

int evcpe_add_object_response_to_xml(
		struct evcpe_add_object_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_ADD_OBJECT_XML_H_ */
