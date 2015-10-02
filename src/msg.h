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

#ifndef EVCPE_MSG_H_
#define EVCPE_MSG_H_

#include <sys/queue.h>

#include <event.h>

#include "minixml.h"
#include "xmlns.h"
#include "xml.h"
#include "method.h"
#include "xml_stack.h"

typedef enum _evcpe_msg_type {
	EVCPE_MSG_UNKNOWN,
	EVCPE_MSG_REQUEST,
	EVCPE_MSG_RESPONSE,
	EVCPE_MSG_FAULT,
} evcpe_msg_type_t;

const char *evcpe_msg_type_to_str(evcpe_msg_type_t type);

typedef struct _evcpe_msg {
	char *session;
	unsigned int major;
	unsigned int minor;
	evcpe_msg_type_t type;
	evcpe_method_type_t method_type;
	void *data;
	int hold_requests;
	int no_more_requests; // CWMP 1.0
} evcpe_msg;

typedef struct _evcpe_msg_parser {
	struct xmlparser xml;
	evcpe_xmlns_table xmlns;
	evcpe_xml_stack stack;
	evcpe_msg *msg;
	tqueue_element *list_item;
} evcpe_msg_parser;

evcpe_msg *evcpe_msg_new(void);

void evcpe_msg_free(evcpe_msg *msg);

int evcpe_msg_from_xml(evcpe_msg *msg, struct evbuffer *buffer);

int evcpe_msg_to_xml(evcpe_msg *msg, struct evbuffer *buffer);

#endif /* EVCPE_MSG_H_ */
