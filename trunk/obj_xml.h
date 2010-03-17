// $Id$

#ifndef EVCPE_OBJ_XML_H_
#define EVCPE_OBJ_XML_H_

#include <event.h>

#include "minixml.h"
#include "xml_stack.h"

#include "obj.h"

struct evcpe_obj_parser {
	struct xmlparser xml;
	struct evcpe_xml_stack stack;
	struct evcpe_obj *root;
};

int evcpe_obj_from_xml(struct evcpe_obj *obj, struct evbuffer *buffer);

int evcpe_obj_to_xml(struct evcpe_obj *obj, struct evbuffer *buffer);

#endif /* EVCPE_OBJ_XML_H_ */
