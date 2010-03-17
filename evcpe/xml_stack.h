// $Id$

#ifndef EVCPE_XML_STACK_H_
#define EVCPE_XML_STACK_H_

#include <sys/queue.h>

struct evcpe_xml_element {
	const char *ns;
	unsigned nslen;
	const char *name;
	unsigned len;
	int ns_declared;
	void *data;
	SLIST_ENTRY(evcpe_xml_element) entry;
};

SLIST_HEAD(evcpe_xml_stack, evcpe_xml_element);

void evcpe_xml_stack_put(struct evcpe_xml_stack *stack,
		struct evcpe_xml_element *elm);

struct evcpe_xml_element *evcpe_xml_stack_peek(
		struct evcpe_xml_stack *stack);

struct evcpe_xml_element *evcpe_xml_stack_pop(struct evcpe_xml_stack *stack);

struct evcpe_xml_element *evcpe_xml_stack_up(struct evcpe_xml_element *elm);

#endif /* EVCPE_XML_STACK_H_ */
