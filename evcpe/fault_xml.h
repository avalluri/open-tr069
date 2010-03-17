// $Id$

#ifndef EVCPE_FAULT_XML_H_
#define EVCPE_FAULT_XML_H_

#include <event.h>

#include "fault.h"

int evcpe_fault_to_xml(struct evcpe_fault *fault, struct evbuffer *buffer);

#endif /* EVCPE_FAULT_XML_H_ */
