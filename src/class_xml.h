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

#ifndef EVCPE_CLASS_XML_H_
#define EVCPE_CLASS_XML_H_

#include <event.h>

#include "minixml.h"
#include "xml_stack.h"

#include "class.h"

typedef struct _evcpe_class_parser {
	struct xmlparser xml;
	evcpe_xml_stack stack;
	evcpe_attr_schema *root;
	void *dynlib;
} evcpe_class_parser;

int evcpe_class_from_xml(evcpe_class *class, struct evbuffer *buffer);

#endif /* EVCPE_CLASS_XML_H_ */
