// $Id$

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
