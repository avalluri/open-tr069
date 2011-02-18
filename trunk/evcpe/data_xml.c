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

#include <errno.h>
#include <stdlib.h>

#include "util.h"
#include "data_xml.h"

int evcpe_device_id_to_xml(struct evcpe_device_id *id,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "Manufacturer", id->manufacturer)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "OUI", id->oui)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "ProductClass", id->product_class)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "SerialNumber", id->serial_number)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_event_to_xml(struct evcpe_event *event,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer,
			"EventCode", event->event_code)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer,
			"CommandKey", event->command_key)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_event_list_to_xml(struct evcpe_event_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_event *event;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":EventStruct[%d]\">\n",
			node, evcpe_event_list_size(list))))
		goto finally;
	TAILQ_FOREACH(event, &list->head, entry) {
		if ((rc = evcpe_event_to_xml(event, "EventStruct",
				buffer)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_param_value_to_xml(struct evcpe_param_value *value,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
//	if ((rc = evcpe_add_buffer(buffer, "<%s xsi:type=\"cwmp:ParameterValueStruct\">\n", node)))
//		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "<Name>%s</Name>\n",
			value->name)))
		goto finally;
//	if ((rc = evcpe_add_buffer(buffer, "<Name xsi:type=\"xsd:string\">%s</Name>\n",
//			value->name)))
//		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "<Value xsi:type=\"xsd:%s\">%.*s</Value>\n",
			evcpe_type_to_str(value->type), value->len, value->data)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_param_value_list_to_xml(struct evcpe_param_value_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_param_value *value;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
			node, evcpe_param_value_list_size(list))))
		goto finally;
	TAILQ_FOREACH(value, &list->head, entry) {
		if ((rc = evcpe_param_value_to_xml(value, "ParameterValueStruct",
				buffer)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_param_info_to_xml(struct evcpe_param_info *info,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_string(buffer, "Name", info->name)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_boolean(buffer, "Writable", info->writable)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_param_info_list_to_xml(struct evcpe_param_info_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_param_info *info;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
			node, evcpe_param_info_list_size(list))))
		goto finally;
	TAILQ_FOREACH(info, &list->head, entry) {
		if ((rc = evcpe_param_info_to_xml(info, "ParameterValueStruct",
				buffer)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_method_list_to_xml(struct evcpe_method_list *list, const char *node,
		struct evbuffer *buffer)
{
	int rc;
	struct evcpe_method_list_item *item;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\"xsd:string[%d]\">\n",
			node, evcpe_method_list_size(list))))
		goto finally;
	TAILQ_FOREACH(item, &list->head, entry) {
		if ((rc = evcpe_xml_add_string(buffer, "string",
				item->name)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;

}
