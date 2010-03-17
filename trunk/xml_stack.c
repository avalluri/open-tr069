// $Id$

#include <stdlib.h>
#include <errno.h>

#include "log.h"

#include "xml_stack.h"

void evcpe_xml_stack_put(struct evcpe_xml_stack *stack,
		struct evcpe_xml_element *elm)
{
	evcpe_trace(__func__, "putting element to stack: %.*s",
			elm->len, elm->name);
	SLIST_INSERT_HEAD(stack, elm, entry);
}

struct evcpe_xml_element *evcpe_xml_stack_peek(
		struct evcpe_xml_stack *stack)
{
	evcpe_trace(__func__, "peeking element from stack");
	return SLIST_FIRST(stack);
}

struct evcpe_xml_element *evcpe_xml_stack_pop(struct evcpe_xml_stack *stack)
{
	struct evcpe_xml_element *elm;

	evcpe_trace(__func__, "popping out element in stack");

	if ((elm = SLIST_FIRST(stack)))
		SLIST_REMOVE_HEAD(stack, entry);
	return elm;
}

struct evcpe_xml_element *evcpe_xml_stack_up(struct evcpe_xml_element *elm)
{
	return SLIST_NEXT(elm, entry);
}
