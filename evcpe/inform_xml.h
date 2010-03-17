// $Id$

#ifndef EVCPE_INFORM_XML_H_
#define EVCPE_INFORM_XML_H_

#include <event.h>

#include "inform.h"

int evcpe_inform_to_xml(struct evcpe_inform *method,
		struct evbuffer *buffer);

#endif /* EVCPE_INFORM_XML_H_ */
