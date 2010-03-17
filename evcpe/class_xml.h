// $Id$

#ifndef EVCPE_CLASS_XML_H_
#define EVCPE_CLASS_XML_H_

#include <event.h>

#include "minixml.h"
#include "xml_stack.h"

#include "class.h"

struct evcpe_class_parser {
	struct xmlparser xml;
	struct evcpe_xml_stack stack;
	struct evcpe_attr_schema *root;
	void *dynlib;
};

int evcpe_class_from_xml(struct evcpe_class *class, struct evbuffer *buffer);

#endif /* EVCPE_CLASS_XML_H_ */
