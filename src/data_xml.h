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

#ifndef DATA_XML_H_
#define DATA_XML_H_

#include <event.h>

#include "xml.h"
#include "data.h"

int evcpe_device_id_to_xml(struct evcpe_device_id *id,
		const char *node, struct evbuffer *buffer);

int evcpe_event_to_xml(struct evcpe_event *event,
		const char *node, struct evbuffer *buffer);

int evcpe_event_list_to_xml(struct evcpe_event_list *list,
		const char *node, struct evbuffer *buffer);

int evcpe_param_value_to_xml(struct evcpe_param_value *value,
		const char *node, struct evbuffer *buffer);

int evcpe_param_value_list_to_xml(struct evcpe_param_value_list *list,
		const char *node, struct evbuffer *buffer);

int evcpe_param_info_list_to_xml(struct evcpe_param_info_list *list,
		const char *node, struct evbuffer *buffer);

int evcpe_method_list_to_xml(struct evcpe_method_list *list,
		const char *node, struct evbuffer *buffer);

#endif /* DATA_XML_H_ */
