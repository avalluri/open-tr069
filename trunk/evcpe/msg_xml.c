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
#include <string.h>

#include "log.h"
#include "util.h"
#include "minixml.h"
#include "fault_xml.h"
#include "inform_xml.h"
#include "get_rpc_methods_xml.h"
#include "get_param_names_xml.h"
#include "get_param_attrs.h"
#include "set_param_attrs.h"
#include "get_param_values_xml.h"
#include "set_param_values_xml.h"
#include "add_object_xml.h"
#include "delete_object.h"

#include "msg_xml.h"

static int evcpe_msg_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_msg_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_msg_xml_data_cb(void *data,
		const char *text, unsigned len);
static int evcpe_msg_xml_attr_cb(void *data,
		const char *ns, unsigned nslen,
		const char *name, unsigned name_len, const char *value, unsigned value_len);

int evcpe_msg_to_xml(struct evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc;
	const char *method = evcpe_method_type_to_str(msg->method_type);

	evcpe_debug(__func__, "marshaling SOAP message");

	if ((rc = evcpe_add_buffer(buffer, "<?xml version=\"1.0\"?>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Envelope "
			"xmlns:"EVCPE_SOAP_ENV_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:"EVCPE_SOAP_ENC_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/encoding/\" "
			"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xmlns:"EVCPE_CWMP_XMLNS
			"=\"urn:dslforum-org:cwmp-%d-%d\">\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_CWMP_XMLNS":ID "
			EVCPE_SOAP_ENV_XMLNS":mustUnderstand=\"1\">%s"
			"</"EVCPE_CWMP_XMLNS":ID>\n"
			"</"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Body>\n",
			msg->major, msg->minor, msg->session))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}
	switch(msg->type) {
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		switch(msg->method_type) {
		case EVCPE_INFORM:
			if ((rc = evcpe_inform_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal inform");
				goto finally;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected request type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	case EVCPE_MSG_RESPONSE:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		switch(msg->method_type) {
		case EVCPE_GET_RPC_METHODS:
			if ((rc = evcpe_get_rpc_methods_response_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal get_rpc_methods_response");
				goto finally;
			}
			break;
		case EVCPE_ADD_OBJECT:
			if ((rc = evcpe_add_object_response_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal add_object_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			if ((rc = evcpe_get_param_values_response_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal get_param_values_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_NAMES:
			if ((rc = evcpe_get_param_names_response_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal get_param_names_response");
				goto finally;
			}
			break;
		case EVCPE_SET_PARAMETER_VALUES:
			if ((rc = evcpe_set_param_values_response_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal set_param_values_response");
				goto finally;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected response type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	case EVCPE_MSG_FAULT:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_SOAP_ENV_XMLNS":Fault>\n"
				"<faultcode>Client</faultcode>\n"
				"<faultstring>CWMP fault</faultstring>\n"
				"<detail>\n"
				"<"EVCPE_CWMP_XMLNS":Fault>\n"))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		if ((rc = evcpe_fault_to_xml(msg->data, buffer))) {
			evcpe_error(__func__, "failed to marshal fault");
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":Fault>\n"
				"</detail>\n"
				"</"EVCPE_SOAP_ENV_XMLNS":Fault>\n"))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	default:
		evcpe_error(__func__, "unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer,
			"</"EVCPE_SOAP_ENV_XMLNS":Body>\n</"EVCPE_SOAP_ENV_XMLNS":Envelope>\n"))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_msg_from_xml(struct evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_msg_parser parser;
	struct evcpe_xml_element *elm;

	evcpe_debug(__func__, "unmarshaling SOAP message");

	if (!EVBUFFER_LENGTH(buffer)) return 0;

	parser.msg = msg;
	parser.xml.data = &parser;
	parser.xml.xmlstart = (const char *)EVBUFFER_DATA(buffer);
	parser.xml.xmlsize = EVBUFFER_LENGTH(buffer);
	parser.xml.starteltfunc = evcpe_msg_xml_elm_begin_cb;
	parser.xml.endeltfunc = evcpe_msg_xml_elm_end_cb;
	parser.xml.datafunc = evcpe_msg_xml_data_cb;
	parser.xml.attfunc = evcpe_msg_xml_attr_cb;
	RB_INIT(&parser.xmlns);
	SLIST_INIT(&parser.stack);
	if ((rc = parsexml(&parser.xml))) {
		evcpe_error(__func__, "failed to parse SOAP message: %d", rc);
	}
	while((elm = evcpe_xml_stack_pop(&parser.stack))) {
		evcpe_error(__func__, "pending stack: %.*s", elm->len, elm->name);
		free(elm);
	}
	evcpe_xmlns_table_clear(&parser.xmlns);
	return rc;
}

int evcpe_msg_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	const char *attr;
	unsigned attr_len;
	const char *urn = "urn:dslforum-org:cwmp-";
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);

	evcpe_trace(__func__, "element begin: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

	if (parent && !parent->ns_declared) {
		evcpe_error(__func__, "parent namespace not declared: %.*s:%.*s",
				parent->nslen, parent->ns, parent->len, parent->name);
		goto syntax_error;
	}
	if (parent && parent->nslen && !nslen) {
		ns = parent->ns;
		nslen = parent->nslen;
	}
	if (!evcpe_strncmp("Envelope", name, len)) {
		if (parent) {
			evcpe_error(__func__, "parent element is not expected");
			goto syntax_error;
		}
	} else if (!parent) {
		evcpe_error(__func__, "parent element is expected");
		goto syntax_error;
	} else if (!evcpe_strncmp("Header", name, len)) {
		if (evcpe_strncmp("Envelope", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("ID", name, len)) {
		if (evcpe_strncmp("Header", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (evcpe_xmlns_table_get(&parser->xmlns, ns, nslen, &attr, &attr_len)) {
			evcpe_error(__func__, "undefined XML namespace: %.*s", nslen, ns);
			return -1;
		}
		// TODO: get cwmp version
	} else if (!evcpe_strncmp("HoldRequests", name, len)) {
		if (evcpe_strncmp("Header", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("NoMoreRequests", name, len)) {
		if (evcpe_strncmp("Header", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("Body", name, len)) {
		if (evcpe_strncmp("Envelope", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("Fault", name, len)) {
		if (!evcpe_strncmp("Body", parent->name, parent->len)) {
			if (!(parser->msg->data = evcpe_fault_new()))
				return ENOMEM;
			parser->msg->type = EVCPE_MSG_FAULT;
		} else if (evcpe_strncmp("detail", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("faultcode", name, len)) {
		if (evcpe_strncmp("Fault", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("faultstring", name, len)) {
		if (evcpe_strncmp("Fault", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("FaultCode", name, len)) {
		if (evcpe_strncmp("Fault", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("FaultString", name, len)) {
		if (evcpe_strncmp("Fault", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("detail", name, len)) {
		if (evcpe_strncmp("Fault", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("GetRPCMethods", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} else if (parent && !evcpe_strncmp("GetRPCMethods", parent->name, parent->len)) {
		evcpe_error(__func__, "unexpected child element");
		goto syntax_error;
	} else if (!evcpe_strncmp("GetParameterNames", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_NAMES;
		if (!(parser->msg->data = evcpe_get_param_names_new()))
			return ENOMEM;
	} else if (!evcpe_strncmp("ParameterPath", name, len)) {
		if (evcpe_strncmp("GetParameterNames", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("NextLevel", name, len)) {
		if (evcpe_strncmp("GetParameterNames", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("GetParameterValues", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_VALUES;
	} else if (!evcpe_strncmp("GetParameterAttributes", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_ATTRIBUTES;
		if (!(parser->msg->data = evcpe_get_param_attrs_new()))
			return ENOMEM;
	} else if (!evcpe_strncmp("ParameterNames", name, len)) {
		if (evcpe_strncmp("GetParameterValues", parent->name, parent->len) &&
				evcpe_strncmp("GetParameterAttributes", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("string", name, len)) {
		if (evcpe_strncmp("ParameterNames", parent->name, parent->len) &&
				evcpe_strncmp("AccessList", parent->name, parent->len) &&
				evcpe_strncmp("MethodList", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("SetParameterValues", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_VALUES;
	} else if (!evcpe_strncmp("SetParameterAttributes", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_attrs_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_ATTRIBUTES;
	} else if (!evcpe_strncmp("ParameterList", name, len)) {
		if (evcpe_strncmp("SetParameterValues", parent->name, parent->len) &&
				evcpe_strncmp("SetParameterAttributes", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("ParameterValueStruct", name, len)) {
		if (evcpe_strncmp("ParameterList", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("SetParameterAttributesStruct", name, len)) {
		if (evcpe_strncmp("ParameterList", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("Name", name, len)) {
		if(evcpe_strncmp("ParameterValueStruct", parent->name, parent->len) &&
				evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("Value", name, len)) {
		if (evcpe_strncmp("ParameterValueStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("NotificationChange", name, len)) {
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("Notification", name, len)) {
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("AccessListChange", name, len)) {
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("AccessList", name, len)) {
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("GetRPCMethodsResponse", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} else if (!evcpe_strncmp("MethodList", name, len)) {
		if (evcpe_strncmp("GetRPCMethodsResponse", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("InformResponse", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_inform_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_INFORM;
	} else if (!evcpe_strncmp("MaxEnvelopes", name, len)) {
		if (evcpe_strncmp("InformResponse", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("AddObject", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_ADD_OBJECT;
	} else if (!evcpe_strncmp("DeleteObject", name, len)) {
		if (evcpe_strncmp("Body", parent->name, parent->len)) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_DELETE_OBJECT;
	} else if (!evcpe_strncmp("ObjectName", name, len)) {
		if (evcpe_strncmp("AddObject", parent->name, parent->len) &&
				evcpe_strncmp("DeleteObject", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} else if (!evcpe_strncmp("ParameterKey", name, len)) {
		if (evcpe_strncmp("AddObject", parent->name, parent->len) &&
				evcpe_strncmp("DeleteObject", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	}
	if (!(parent = calloc(1, sizeof(struct evcpe_xml_element)))) {
		evcpe_error(__func__, "failed to calloc evcpe_soap_element");
		return ENOMEM;
	}
	parent->ns = ns;
	parent->nslen = nslen;
	parent->name = name;
	parent->len = len;
	evcpe_xml_stack_put(&parser->stack, parent);
	if (nslen && evcpe_xmlns_table_find(&parser->xmlns, ns, nslen))
		parent->ns_declared = 1;
	return 0;

unexpected_parent:
	evcpe_error(__func__, "unexpected parent element");

syntax_error:
	evcpe_error(__func__, "syntax error");
	return EPROTO;
}

int evcpe_msg_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_get_param_values *get_params;
	struct evcpe_set_param_values *set_params;
	struct evcpe_get_param_attrs *get_attrs;

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) return -1;

	evcpe_trace(__func__, "element end: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

	if ((nslen && evcpe_strcmp(elm->ns, elm->nslen, ns, nslen)) ||
			evcpe_strcmp(elm->name, elm->len, name, len)) {
		evcpe_error(__func__, "element doesn't match start: %.*s:%.*s",
				nslen, ns, len, name);
		rc = EPROTO;
		goto finally;
	}

	if (!evcpe_strncmp("Header", name, len)) {
		if (!parser->msg->session)
			goto syntax_error;
	} else if (!evcpe_strncmp("Header", name, len)) {
		if (!parser->msg->session)
			goto syntax_error;
	} else if (!evcpe_strncmp("Body", name, len)) {
		if (!parser->msg->data)
			goto syntax_error;
	} else if (!evcpe_strncmp("GetParameterValues", name, len)) {
		get_params = parser->msg->data;
		if (!evcpe_param_name_list_size(&get_params->parameter_names))
			goto syntax_error;
	} else if (!evcpe_strncmp("SetParameterValues", name, len)) {
		set_params = parser->msg->data;
		if (!evcpe_set_param_value_list_size(&set_params->parameter_list))
			goto syntax_error;
	} else if (!evcpe_strncmp("GetParameterAttributes", name, len)) {
		get_attrs = parser->msg->data;
		if (!evcpe_param_name_list_size(&get_attrs->parameter_names))
			goto syntax_error;
	}
	rc = 0;
	goto finally;

syntax_error:
	evcpe_error(__func__, "syntax error");
	rc = EPROTO;

finally:
	free(elm);
	return rc;
}

int evcpe_msg_xml_data_cb(void *data, const char *text, unsigned len)
{
	int rc;
	long val;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_fault *fault;
	struct evcpe_get_param_names *get_names;
	struct evcpe_get_param_values *get_params;
	struct evcpe_set_param_values *set_params;
	struct evcpe_get_param_attrs *get_attrs;
	struct evcpe_set_param_attrs *set_attrs;
	struct evcpe_param_name *param_name;
	struct evcpe_set_param_value *param_value;
	struct evcpe_set_param_attr *param_attr;
	struct evcpe_access_list_item *item;
	struct evcpe_get_rpc_methods_response *get_methods_resp;
	struct evcpe_method_list_item *method_item;
	struct evcpe_inform_response *inform_resp;
	struct evcpe_add_object *add_obj;
	struct evcpe_delete_object *delete_obj;

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) return -1;

	evcpe_trace(__func__, "text: %.*s", len, text);

	if (!evcpe_strncmp("ID", elm->name, elm->len)) {
		if (!(parser->msg->session = malloc(len + 1))) {
			rc = ENOMEM;
			goto finally;
		}
		memcpy(parser->msg->session, text, len);
		parser->msg->session[len] = '\0';
	} else if (!evcpe_strncmp("HoldRequests", elm->name, elm->len)) {
		if (len == 1) {
			if (text[0] == '0')
				parser->msg->hold_requests = 0;
			else if (text[0] == '1')
				parser->msg->hold_requests = 1;
			else
				goto syntax_error;
		} else {
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("NoMoreRequests", elm->name, elm->len)) {
		if (len == 1) {
			if (text[0] == '0')
				parser->msg->no_more_requests = 0;
			else if (text[0] == '1')
				parser->msg->no_more_requests = 1;
			else
				goto syntax_error;
		} else {
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("faultcode", elm->name, elm->len)) {
	} else if (!evcpe_strncmp("faultstring", elm->name, elm->len)) {
		if (evcpe_strncmp("CWMP fault", text, len))
			goto syntax_error;
	} else if (!evcpe_strncmp("FaultCode", elm->name, elm->len)) {
		fault = parser->msg->data;
		if ((rc = evcpe_atol(text, len, &val))) {
			evcpe_error(__func__, "failed to convert to "
					"integer: %.*s", len, text);
			goto finally;
		}
		fault->code = val;
	} else if (!evcpe_strncmp("FaultString", elm->name, elm->len)) {
		fault = parser->msg->data;
		if (len >= sizeof(fault->string))
			return EOVERFLOW;
		memcpy(fault->string, text, len);
		fault->string[len] = '\0';
	} else if (!evcpe_strncmp("string", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_RPC_METHODS:
			get_methods_resp = parser->msg->data;
			if ((rc = evcpe_method_list_add(&get_methods_resp->method_list,
					&method_item, text, len)))
				goto finally;
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			get_params = parser->msg->data;
			if ((rc = evcpe_param_name_list_add(&get_params->parameter_names,
					&param_name, text, len)))
				goto finally;
			break;
		case EVCPE_GET_PARAMETER_ATTRIBUTES:
			get_attrs = parser->msg->data;
			if ((rc = evcpe_param_name_list_add(&get_attrs->parameter_names,
					&param_name, text, len)))
				goto finally;
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if ((rc = evcpe_access_list_add(&param_attr->access_list,
					&item, text, len)))
				goto finally;
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("ParameterPath", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_PARAMETER_NAMES:
			get_names = parser->msg->data;
			if (len >= sizeof(get_names->parameter_path))
				goto syntax_error;
			strncpy(get_names->parameter_path, text, len);
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("NextLevel", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_PARAMETER_NAMES:
			get_names = parser->msg->data;
			if ((rc = evcpe_atol(text, len, &val)))
				goto syntax_error;
			get_names->next_level = val;
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("Name", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			set_params = parser->msg->data;
			if ((rc = evcpe_set_param_value_list_add(&set_params->parameter_list,
					&param_value, text, len)))
				goto finally;
			parser->list_item = param_value;
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			set_attrs = parser->msg->data;
			if ((rc = evcpe_set_param_attr_list_add(&set_attrs->parameter_list,
					&param_attr, text, len)))
				goto finally;
			parser->list_item = param_attr;
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("Value", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			param_value = parser->list_item;
			if ((rc = evcpe_set_param_value_set(param_value, text, len)))
				goto finally;
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("NotificationChange", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len == 1) {
				if (text[0] == '0')
					param_attr->notification_change = 0;
				else if (text[0] == '1')
					param_attr->notification_change = 1;
				else
					goto syntax_error;
			} else {
				goto syntax_error;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("Notification", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len == 1) {
				if (text[0] == '0')
					param_attr->notification = 0;
				else if (text[0] == '1')
					param_attr->notification = 1;
				else if (text[0] == '2')
					param_attr->notification = 2;
				else
					goto syntax_error;
			} else {
				goto syntax_error;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("AccessListChange", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len == 1) {
				if (text[0] == '0')
					param_attr->access_list_change = 0;
				else if (text[0] == '1')
					param_attr->access_list_change = 1;
				else
					goto syntax_error;
			} else {
				goto syntax_error;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("AccessList", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len && !strcmp("1", text))
				param_attr->access_list_change = 1;
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("MaxEnvelopes", elm->name, elm->len)) {
		if (len != 1 || text[0] != '1')
			goto syntax_error;
		inform_resp = parser->msg->data;
		inform_resp->max_envelopes = 1;
	} else if (!evcpe_strncmp("ObjectName", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_ADD_OBJECT:
			add_obj = parser->msg->data;
			if (len >= sizeof(add_obj->object_name)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(add_obj->object_name, text, len);
			add_obj->object_name[len] = '\0';
			break;
		case EVCPE_DELETE_OBJECT:
			delete_obj = parser->msg->data;
			if (len >= sizeof(delete_obj->object_name)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(delete_obj->object_name, text, len);
			delete_obj->object_name[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (!evcpe_strncmp("ParameterKey", elm->name, elm->len)) {
		switch (parser->msg->method_type) {
		case EVCPE_ADD_OBJECT:
			add_obj = parser->msg->data;
			if (len >= sizeof(add_obj->parameter_key)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(add_obj->parameter_key, text, len);
			add_obj->parameter_key[len] = '\0';
			break;
		case EVCPE_DELETE_OBJECT:
			delete_obj = parser->msg->data;
			if (len >= sizeof(delete_obj->parameter_key)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(delete_obj->parameter_key, text, len);
			delete_obj->parameter_key[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (len > 0) {
		evcpe_error(__func__, "unexpected element: %.*s",
				elm->len, elm->name);
		goto syntax_error;
	}
	rc = 0;

finally:
	return rc;

syntax_error:
	evcpe_error(__func__, "syntax error");
	return EPROTO;
}

int evcpe_msg_xml_attr_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);

	evcpe_trace(__func__, "attribute: %.*s => %.*s (namespace: %.*s)",
			name_len, name, value_len, value, nslen, ns);

	if (nslen && !evcpe_strncmp("xmlns", ns, nslen)) {
		if ((rc = evcpe_xmlns_table_add(&parser->xmlns,
				name, name_len, value, value_len)))
			goto finally;
		if (!evcpe_strcmp(parent->ns, parent->nslen, name, name_len))
			parent->ns_declared = 1;
//	} else {
//		evcpe_error(__func__, "unexpected attribute: %.*s",
//				name_len, name);
//		rc = EPROTO;
//		goto finally;
	}
	rc = 0;

finally:
	return rc;
}
