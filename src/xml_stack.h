// $Id$
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EVCPE_XML_STACK_H_
#define EVCPE_XML_STACK_H_

#include <sys/queue.h>

typedef struct _evcpe_xml_element {
	const char *ns;
	unsigned nslen;
	const char *name;
	unsigned len;
	int ns_declared;
	void *data;
	SLIST_ENTRY(_evcpe_xml_element) entry;
} evcpe_xml_element;

SLIST_HEAD(_evcpe_xml_stack, _evcpe_xml_element);
typedef struct _evcpe_xml_stack evcpe_xml_stack;

void evcpe_xml_stack_put(evcpe_xml_stack *stack,
		evcpe_xml_element *elm);

evcpe_xml_element *evcpe_xml_stack_peek(evcpe_xml_stack *stack);

evcpe_xml_element *evcpe_xml_stack_pop(evcpe_xml_stack *stack);

evcpe_xml_element *evcpe_xml_stack_up(evcpe_xml_element *elm);

#endif /* EVCPE_XML_STACK_H_ */
