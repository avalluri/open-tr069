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
