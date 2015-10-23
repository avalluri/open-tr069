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
#include "evcpe-config.h"
#include "add_object.h"
#include "delete_object.h"
#include "fault.h"
#include "inform.h"
#include "get_rpc_methods.h"
#include "get_param_names.h"
#include "get_param_attrs.h"
#include "set_param_attrs.h"
#include "get_param_values.h"
#include "set_param_values.h"
#include "download.h"

#include "msg.h"

const char *evcpe_msg_type_to_str(evcpe_msg_type_t type)
{
	switch (type) {
	case EVCPE_MSG_REQUEST:
		return "request";
	case EVCPE_MSG_RESPONSE:
		return "response";
	case EVCPE_MSG_FAULT:
		return "fault";
	default:
		return "unknown";
	}
}

evcpe_msg *evcpe_msg_new(void)
{
	evcpe_msg *msg;

	DEBUG("constructing evcpe_msg");

	if (!(msg = calloc(1, sizeof(evcpe_msg)))) {
		ERROR("failed to calloc evcpe_msg");
		return NULL;
	}
	msg->major = 1;
	msg->minor = 0;
	return msg;
}

void evcpe_msg_free(evcpe_msg *msg)
{
	if (!msg) return;

	DEBUG("destructing evcpe_msg");

	if (msg->data) {
		switch(msg->type) {
		case EVCPE_MSG_UNKNOWN:
			break;
		case EVCPE_MSG_REQUEST:
			switch(msg->method_type) {
			case EVCPE_GET_RPC_METHODS:
				evcpe_get_rpc_methods_free(msg->data);
				break;
			case EVCPE_GET_PARAMETER_NAMES:
				evcpe_get_param_names_free(msg->data);
				break;
			case EVCPE_GET_PARAMETER_VALUES:
				evcpe_get_param_values_free(msg->data);
				break;
			case EVCPE_SET_PARAMETER_VALUES:
				evcpe_set_param_values_free(msg->data);
				break;
			case EVCPE_GET_PARAMETER_ATTRIBUTES:
				evcpe_get_param_attrs_free(msg->data);
				break;
			case EVCPE_SET_PARAMETER_ATTRIBUTES:
				evcpe_set_param_attrs_free(msg->data);
				break;
			case EVCPE_ADD_OBJECT:
				evcpe_add_object_free(msg->data);
				break;
			case EVCPE_DELETE_OBJECT:
				evcpe_delete_object_free(msg->data);
				break;
			case EVCPE_DOWNLOAD:
				evcpe_download_free(msg->data);
				break;
			case EVCPE_INFORM:
				evcpe_inform_free(msg->data);
				break;
			default:
				ERROR("unexpected request type: %d", msg->method_type); break;
			}
			break;
		case EVCPE_MSG_RESPONSE:
			switch(msg->method_type) {
			case EVCPE_GET_RPC_METHODS:
				evcpe_get_rpc_methods_response_free(msg->data);
				break;
			case EVCPE_ADD_OBJECT:
				evcpe_add_object_response_free(msg->data);
				break;
			case EVCPE_GET_PARAMETER_NAMES:
				evcpe_get_param_names_response_free(msg->data);
				break;
			case EVCPE_GET_PARAMETER_VALUES:
				evcpe_get_param_values_response_free(msg->data);
				break;
			case EVCPE_SET_PARAMETER_VALUES:
				evcpe_set_param_values_response_free(msg->data);
				break;
			case EVCPE_INFORM:
				evcpe_inform_response_free(msg->data);
				break;
			default:
				ERROR("unexpected response type: %d", msg->method_type); break;
			}
			break;
		case EVCPE_MSG_FAULT:
			evcpe_fault_free(msg->data);
			break;
		default:
			ERROR("unexpected message type: %d", msg->type); break;
		}
	}
	if (msg->session) free(msg->session);
	free(msg);
}

int evcpe_msg_to_xml(evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc;
	const char *method = evcpe_method_type_to_str(msg->method_type);

	DEBUG("marshaling SOAP message");

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
		ERROR("failed to append buffer");
		goto finally;
	}
	switch(msg->type) {
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		switch(msg->method_type) {
		case EVCPE_INFORM:
			if ((rc = evcpe_inform_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal inform");
				goto finally;
			}
			break;
		default:
			ERROR("unexpected request type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		break;
	case EVCPE_MSG_RESPONSE:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		switch(msg->method_type) {
		case EVCPE_GET_RPC_METHODS:
			if ((rc = evcpe_get_rpc_methods_response_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal get_rpc_methods_response");
				goto finally;
			}
			break;
		case EVCPE_ADD_OBJECT:
			if ((rc = evcpe_add_object_response_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal add_object_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			if ((rc = evcpe_get_param_values_response_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal get_param_values_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_NAMES:
			if ((rc = evcpe_get_param_names_response_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal get_param_names_response");
				goto finally;
			}
			break;
		case EVCPE_SET_PARAMETER_VALUES:
			if ((rc = evcpe_set_param_values_response_to_xml(msg->data, buffer))) {
				ERROR("failed to marshal set_param_values_response");
				goto finally;
			}
			break;
		default:
			ERROR("unexpected response type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		break;
	case EVCPE_MSG_FAULT:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_SOAP_ENV_XMLNS":Fault>\n"
				"<faultcode>Client</faultcode>\n"
				"<faultstring>CWMP fault</faultstring>\n"
				"<detail>\n"
				"<"EVCPE_CWMP_XMLNS":Fault>\n"))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		if ((rc = evcpe_fault_to_xml(msg->data, buffer))) {
			ERROR("failed to marshal fault");
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":Fault>\n"
				"</detail>\n"
				"</"EVCPE_SOAP_ENV_XMLNS":Fault>\n"))) {
			ERROR("failed to append buffer");
			goto finally;
		}
		break;
	default:
		ERROR("unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer,
			"</"EVCPE_SOAP_ENV_XMLNS":Body>\n</"EVCPE_SOAP_ENV_XMLNS":Envelope>\n"))) {
		ERROR("failed to append buffer");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

typedef enum {
	NODE_Unknown_,
	NODE_Envelope,
	NODE_Header,
	NODE_ID,
	NODE_HoldRequests,
	NODE_NoMoreRequests,
	NODE_Body,
	NODE_Fault,
	NODE_Deatail,
	NODE_faultcode,
	NODE_faultstring,
	NODE_FaultCode,
	NODE_FaultString,
	NODE_detail,
	NODE_GetRPCMethods,
	NODE_GetParameterNames,
	NODE_ParameterPath,
	NODE_NextLevel,
	NODE_GetParameterValues,
	NODE_GetParameterAttributes,
	NODE_ParameterNames,
	NODE_string,
	NODE_MethodList,
	NODE_SetParameterValues,
	NODE_SetParameterAttributes,
	NODE_ParameterList,
	NODE_ParameterValueStruct,
	NODE_ParameterKey,
	NODE_AddObject,
	NODE_DeleteObject,
	NODE_SetParameterAttributesStruct,
	NODE_Name,
	NODE_Value,
	NODE_NotificationChange,
	NODE_Notification,
	NODE_AccessListChange,
	NODE_AccessList,
	NODE_GetRPCMethodsResponse,
	NODE_InformResponse,
	NODE_MaxEnvelopes,
	NODE_ObjectName,
	NODE_Download,
	NODE_CommandKey,
	NODE_FileType,
	NODE_URL,
	NODE_Username,
	NODE_Passwrod,
	NODE_FileSize,
	NODE_TargetFileName,
	NODE_DelaySeconds,
	NODE_SuccessURL,
	NODE_FailureURL,
} node_type_t;

typedef struct _msg_node {
	node_type_t type;
	char name[256];
	RB_ENTRY(_msg_node) entry;
} msg_node;

typedef RB_HEAD(_msg_tree, _msg_node) msg_tree;

int _msg_node_cmp(msg_node *a, msg_node* b)
{
	return strcmp(a->name, b->name);
}

RB_PROTOTYPE(_msg_tree, _msg_node, entry, _msg_node_cmp);
RB_GENERATE(_msg_tree, _msg_node, entry, _msg_node_cmp);

static msg_tree _tree;
static int      _tree_ready;

static
void _msg_tree_init()
{
	static msg_node msg_nodes[] = {
		{ NODE_Envelope, "Envelope" },
		{ NODE_Header, "Header" },
		{ NODE_ID, "ID" },
		{ NODE_HoldRequests, "HoldRequests" },
		{ NODE_NoMoreRequests, "NoMoreRequests" },
		{ NODE_Body, "Body" },
		{ NODE_Fault, "Fault" },
		{ NODE_faultcode, "faultcode" },
		{ NODE_faultstring, "faultstring" },
		{ NODE_FaultCode, "FaultCode" },
		{ NODE_FaultString, "FaultString" },
		{ NODE_detail, "detail" },
		{ NODE_GetRPCMethods, "GetRPCMethods"},
		{ NODE_GetParameterNames, "GetParameterNames" },
		{ NODE_ParameterPath, "ParameterPath" },
		{ NODE_NextLevel, "NextLevel" },
		{ NODE_GetParameterValues, "GetParameterValues" },
		{ NODE_GetParameterAttributes, "GetParameterAttributes" },
		{ NODE_ParameterNames, "ParameterNames" },
		{ NODE_string, "string" },
		{ NODE_MethodList, "MethodList" },
		{ NODE_SetParameterValues, "SetParameterValues" },
		{ NODE_SetParameterAttributes, "SetParameterAttributes" },
		{ NODE_ParameterList, "ParameterList" },
		{ NODE_ParameterValueStruct, "ParameterValueStruct" },
		{ NODE_ParameterKey, "ParameterKey" },
		{ NODE_AddObject, "AddObject" },
		{ NODE_DeleteObject, "DeleteObject" },
		{ NODE_SetParameterAttributesStruct, "SetParameterAttributesStruct" },
		{ NODE_Name, "Name" },
		{ NODE_Value, "Value" },
		{ NODE_NotificationChange, "NotificationChange" },
		{ NODE_Notification, "Notification" },
		{ NODE_AccessListChange, "AccessListChange" },
		{ NODE_AccessList, "AccessList" },
		{ NODE_GetRPCMethodsResponse, "GetRPCMethodsResponse" },
		{ NODE_InformResponse, "InformResponse" },
		{ NODE_MaxEnvelopes, "MaxEnvelopes" },
		{ NODE_ObjectName, "ObjectName" },
		{ NODE_Download, "Download" },
		{ NODE_CommandKey, "CommandKey" },
		{ NODE_FileType, "FileType" },
		{ NODE_URL, "URL" },
		{ NODE_Username, "Username" },
		{ NODE_Passwrod, "Password" },
		{ NODE_FileSize, "FileSize" },
		{ NODE_TargetFileName, "TargetFileName" },
		{ NODE_DelaySeconds, "DelaySeconds" },
		{ NODE_SuccessURL, "SuccessURL" },
		{ NODE_FailureURL, "FailureURL" }
	};
	size_t i, max = sizeof(msg_nodes) / sizeof(*msg_nodes);
	RB_INIT(&_tree);

	if (_tree_ready) return;

	for(i = 0; i < max; i++) {
		RB_INSERT(_msg_tree, &_tree, &msg_nodes[i]);
	}
	_tree_ready = 1;
}

static
node_type_t _get_node_type(const char* name, unsigned len)
{
	msg_node elm, *found;
	elm.type = NODE_Unknown_;
	snprintf(elm.name, sizeof(elm.name), "%.*s", len, name);

	found = RB_FIND(_msg_tree, &_tree, &elm);

	return found ? found->type : NODE_Unknown_;
}

static
int evcpe_msg_xml_elm_begin_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned len)
{
	const char *attr;
	unsigned attr_len;
	evcpe_xml_element* elm = NULL;
	evcpe_msg_parser *parser = data;
	evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);
	node_type_t type, parent_type;

	TRACE("element begin: %.*s (namespace: %.*s)", len, name, nslen, ns);

	if (parent && !parent->ns_declared) {
		ERROR("parent namespace not declared: %.*s:%.*s",
				parent->nslen, parent->ns, parent->len, parent->name);
		goto syntax_error;
	}
	if (parent && parent->nslen && !nslen) {
		ns = parent->ns;
		nslen = parent->nslen;
	}

	type = _get_node_type(name, len);
	if (type == NODE_Unknown_) {
		ERROR("Unknown element %.*s", len, name);
		goto syntax_error;
	}
	if (parent)	parent_type = (node_type_t) parent->data;

	if (type == NODE_Envelope) {
		if (parent) {
			ERROR("parent element is not expected");
			goto syntax_error;
		}
	} else if (!parent) {
		ERROR("parent element is expected");
		goto syntax_error;
	} else if (type == NODE_Header) {
		if (parent_type != NODE_Envelope) {
			goto unexpected_parent;
		}
	} else if (type == NODE_ID) {
		if (parent_type != NODE_Header) {
			goto unexpected_parent;
		}
		if (evcpe_xmlns_table_get(&parser->xmlns, ns, nslen, &attr, &attr_len)) {
			ERROR("undefined XML namespace: %.*s", nslen, ns);
			return -1;
		}
		// TODO: get cwmp version
	} else if (type == NODE_HoldRequests) {
		if (parent_type != NODE_Header) {
			goto unexpected_parent;
		}
	} else if (type == NODE_NoMoreRequests) {
		if (parent_type != NODE_Header) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Body) {
		if (parent_type != NODE_Envelope) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Fault) {
		if (parent_type == NODE_Body) {
			if (!(parser->msg->data = evcpe_fault_new()))
				return ENOMEM;
			parser->msg->type = EVCPE_MSG_FAULT;
		} else if (parent_type != NODE_detail) {
			goto unexpected_parent;
		}
	} else if (type == NODE_faultcode) {
		if (parent_type != NODE_Fault) {
			goto unexpected_parent;
		}
	} else if (type == NODE_faultstring) {
		if (parent_type != NODE_Fault) {
			goto unexpected_parent;
		}
	} else if (type == NODE_FaultCode) {
		if (parent_type != NODE_Fault) {
			goto unexpected_parent;
		}
	} else if (type == NODE_FaultString) {
		if (parent_type != NODE_Fault) {
			goto unexpected_parent;
		}
	} else if (type == NODE_detail) {
		if (parent_type != NODE_Fault) {
			goto unexpected_parent;
		}
	} else if (type == NODE_GetRPCMethods) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} else if (parent && parent_type == NODE_GetRPCMethods) {
		ERROR("unexpected child element");
		goto syntax_error;
	} else if (type == NODE_GetParameterNames) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_NAMES;
		if (!(parser->msg->data = evcpe_get_param_names_new()))
			return ENOMEM;
	} else if (type == NODE_ParameterPath) {
		if (parent_type != NODE_GetParameterNames) {
			goto unexpected_parent;
		}
	} else if (type == NODE_NextLevel) {
		if (parent_type != NODE_GetParameterNames) {
			goto unexpected_parent;
		}
	} else if (type == NODE_GetParameterValues) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_VALUES;
	} else if (type == NODE_GetParameterAttributes) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_ATTRIBUTES;
		if (!(parser->msg->data = evcpe_get_param_attrs_new()))
			return ENOMEM;
	} else if (type == NODE_ParameterNames) {
		if (parent_type != NODE_GetParameterValues &&
			parent_type != NODE_GetParameterAttributes) {
			goto unexpected_parent;
		}
	} else if (type == NODE_string) {
		if (parent_type != NODE_ParameterNames &&
			parent_type != NODE_AccessList &&
			parent_type != NODE_MethodList) {
			goto unexpected_parent;
		}
	} else if (type == NODE_SetParameterValues) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_VALUES;
	} else if (type == NODE_SetParameterAttributes) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_attrs_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_ATTRIBUTES;
	} else if (type == NODE_ParameterList) {
		if (parent_type != NODE_SetParameterValues &&
			parent_type != NODE_SetParameterAttributes) {
			goto unexpected_parent;
		}
	} else if (type == NODE_ParameterValueStruct) {
		if (parent_type != NODE_ParameterList) {
			goto unexpected_parent;
		}
	} else if (type == NODE_ParameterKey) {
		if (parent_type != NODE_SetParameterValues &&
			parent_type != NODE_AddObject &&
			parent_type != NODE_DeleteObject)
			goto unexpected_parent;
	} else if (type == NODE_SetParameterAttributesStruct) {
		if (parent_type != NODE_ParameterList) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Name) {
		if( parent_type != NODE_ParameterValueStruct &&
			parent_type != NODE_SetParameterAttributesStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Value) {
		if (parent_type != NODE_ParameterValueStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_NotificationChange) {
		if (parent_type != NODE_SetParameterAttributesStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Notification) {
		if (parent_type != NODE_SetParameterAttributesStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_AccessListChange) {
		if (parent_type != NODE_SetParameterAttributesStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_AccessList) {
		if (parent_type != NODE_SetParameterAttributesStruct) {
			goto unexpected_parent;
		}
	} else if (type == NODE_GetRPCMethodsResponse) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} else if (type == NODE_MethodList) {
		if (parent_type != NODE_GetRPCMethodsResponse) {
			goto unexpected_parent;
		}
	} else if (type == NODE_InformResponse) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_inform_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_INFORM;
	} else if (type == NODE_MaxEnvelopes) {
		if (parent_type != NODE_InformResponse) {
			goto unexpected_parent;
		}
	} else if (type == NODE_AddObject) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_ADD_OBJECT;
	} else if (type == NODE_DeleteObject) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_DELETE_OBJECT;
	} else if (type == NODE_ObjectName) {
		if (parent_type != NODE_AddObject &&
			parent_type != NODE_DeleteObject) {
			goto unexpected_parent;
		}
	} else if (type == NODE_ParameterKey) {
		if (parent_type != NODE_SetParameterValues &&
			parent_type != NODE_AddObject &&
			parent_type != NODE_DeleteObject) {
			goto unexpected_parent;
		}
	} else if (type == NODE_Download) {
		if (parent_type != NODE_Body) {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_download_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_DOWNLOAD;
	} else if (type == NODE_FileType ||
			   type == NODE_URL ||
			   type == NODE_Username ||
			   type == NODE_Passwrod ||
			   type == NODE_FileSize ||
			   type == NODE_TargetFileName ||
			   type == NODE_DelaySeconds ||
			   type == NODE_SuccessURL ||
			   type == NODE_FailureURL) {
		if (parent_type != NODE_Download) {
			goto unexpected_parent;
		}
	}

	if (!(elm = calloc(1, sizeof(evcpe_xml_element)))) {
		ERROR("failed to calloc evcpe_soap_element");
		return ENOMEM;
	}

	elm->ns = ns;
	elm->nslen = nslen;
	elm->name = name;
	elm->len = len;
	elm->data = (void*)type;
	evcpe_xml_stack_put(&parser->stack, elm);
	if (nslen && evcpe_xmlns_table_find(&parser->xmlns, ns, nslen))
		elm->ns_declared = 1;
	return 0;

unexpected_parent:
	ERROR("unexpected parent element");

syntax_error:
	ERROR("syntax error");
	return EPROTO;
}

static
int evcpe_msg_xml_elm_end_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned len)
{
	int rc;
	evcpe_msg_parser *parser = data;
	evcpe_xml_element *elm;
	node_type_t elm_type;

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) return -1;

	TRACE("element end: %.*s (namespace: %.*s)", len, name, nslen, ns);

	if ((nslen && evcpe_strcmp(elm->ns, elm->nslen, ns, nslen)) ||
			evcpe_strcmp(elm->name, elm->len, name, len)) {
		ERROR("element doesn't match start: %.*s:%.*s", nslen, ns, len, name);
		rc = EPROTO;
		goto finally;
	}

	elm_type = (node_type_t)elm->data;

	if (elm_type == NODE_Header) {
		if (!parser->msg->session)
			goto syntax_error;
	} else if (elm_type == NODE_Body) {
		if (!parser->msg->data)
			goto syntax_error;
	} else if (elm_type == NODE_GetParameterValues) {
		evcpe_get_param_values *get_params = parser->msg->data;
		if (!tqueue_size(get_params->parameter_names))
			goto syntax_error;
	} else if (elm_type == NODE_SetParameterValues) {
		evcpe_set_param_values *set_params = parser->msg->data;
		if (!tqueue_size(set_params->parameter_list))
			goto syntax_error;
	} else if (elm_type == NODE_GetParameterAttributes) {
		evcpe_get_param_attrs *get_attrs = parser->msg->data;
		if (!tqueue_size(get_attrs->parameter_names))
			goto syntax_error;
	}
	rc = 0;
	goto finally;

syntax_error:
	ERROR("syntax error");
	rc = EPROTO;

finally:
	free(elm);
	return rc;
}

static
int evcpe_msg_xml_data_cb(void *data, const char *text, unsigned len)
{
	int rc;
	long val;
	evcpe_msg_parser *parser = data;
	evcpe_xml_element *elm;
	node_type_t elm_type;
	evcpe_fault *fault;
	evcpe_get_param_names *get_names;
	evcpe_get_param_values *get_params;
	evcpe_set_param_values *set_params;
	evcpe_get_param_attrs *get_attrs;
	evcpe_set_param_attrs *set_attrs;
	evcpe_param_value *param_value;
	evcpe_set_param_attr *param_attr;
	evcpe_get_rpc_methods_response *get_methods_resp;
	evcpe_inform_response *inform_resp;
	evcpe_add_object *add_obj;
	evcpe_delete_object *delete_obj;
	evcpe_download* download = NULL;

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) return -1;

	TRACE("text: %.*s", len, text);

	elm_type = (node_type_t)elm->data;

	if (elm_type == NODE_ID) {
		if (!(parser->msg->session = malloc(len + 1))) {
			rc = ENOMEM;
			goto finally;
		}
		memcpy(parser->msg->session, text, len);
		parser->msg->session[len] = '\0';
	} else if (elm_type == NODE_HoldRequests) {
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
	} else if (elm_type == NODE_NoMoreRequests) {
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
	} else if (elm_type == NODE_faultcode) {
	} else if (elm_type == NODE_faultstring) {
		if (evcpe_strncmp("CWMP fault", text, len))
			goto syntax_error;
	} else if (elm_type == NODE_FaultCode) {
		fault = parser->msg->data;
		if ((rc = evcpe_atol(text, len, &val))) {
			ERROR("failed to convert to integer: %.*s", len, text);
			goto finally;
		}
		fault->code = val;
	} else if (elm_type == NODE_FaultString) {
		fault = parser->msg->data;
		if (len >= sizeof(fault->string))
			return EOVERFLOW;
		memcpy(fault->string, text, len);
		fault->string[len] = '\0';
	} else if (elm_type == NODE_string) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_RPC_METHODS:
			get_methods_resp = parser->msg->data;
			tqueue_insert(get_methods_resp->method_list,
					(void*)evcpe_method_type_from_str(text, len));
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			get_params = parser->msg->data;
			if (!tqueue_insert(get_params->parameter_names,
					(void*)evcpe_strdup(text, len))) {
				rc = -1; goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_ATTRIBUTES:
			get_attrs = parser->msg->data;
			if (!tqueue_insert(get_attrs->parameter_names,
					(void*)evcpe_strdup(text, len))) {
				rc = -1; goto finally;
			}
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = (evcpe_set_param_attr*)parser->list_item->data;
			tqueue_insert(param_attr->info->access_list,
					(void*)evcpe_strdup(text, len));
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_ParameterPath) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_PARAMETER_NAMES:
			get_names = parser->msg->data;
			if (len >= sizeof(get_names->parameter_path))
				goto syntax_error;
			strncpy(get_names->parameter_path, text, len);
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_NextLevel) {
		switch (parser->msg->method_type) {
		case EVCPE_GET_PARAMETER_NAMES:
			get_names = parser->msg->data;
			if ((rc = evcpe_atol(text, len, &val)))
				goto syntax_error;
			get_names->next_level = val;
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d",
					parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_Name) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			set_params = parser->msg->data;
			parser->list_item = tqueue_insert(set_params->parameter_list,
					(void*)evcpe_param_value_new(text, len, NULL, 0,
							EVCPE_TYPE_UNKNOWN));
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			set_attrs = parser->msg->data;
			parser->list_item = tqueue_insert(set_attrs->parameter_list,
					(void*)evcpe_set_param_attr_new(text, len));
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_Value) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			param_value = (evcpe_param_value*)parser->list_item->data;
			param_value->data = text;
			param_value->len = len;
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_NotificationChange) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = (evcpe_set_param_attr*)parser->list_item->data;
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
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_Notification) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = (evcpe_set_param_attr*)parser->list_item->data;
			if (len == 1) {
				int n = text[0] - '0';
				if (n >= EVCPE_NOTIFICATION_OFF ||
					n < EVCPE_NOTIFICATION_UNKNOWN)
					param_attr->info->notification = n;
				else
					goto syntax_error;
			} else
				goto syntax_error;
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_AccessListChange) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = (evcpe_set_param_attr*)parser->list_item->data;
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
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_AccessList) {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = (evcpe_set_param_attr*)parser->list_item->data;
			if (len == 1 && text[0] == '1')
				param_attr->access_list_change = 1;
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_MaxEnvelopes) {
		if (len != 1 || text[0] != '1')
			goto syntax_error;
		inform_resp = parser->msg->data;
		inform_resp->max_envelopes = 1;
	} else if (elm_type == NODE_ObjectName) {
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
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}
	} else if (elm_type == NODE_ParameterKey) {
		char *key_holder = NULL;
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			set_params = parser->msg->data;
			key_holder = set_params->parameter_key;
			break;
		case EVCPE_ADD_OBJECT:
			add_obj = parser->msg->data;
			key_holder = add_obj->parameter_key;
			break;
		case EVCPE_DELETE_OBJECT:
			delete_obj = parser->msg->data;
			key_holder = delete_obj->parameter_key;
			break;
		default:
			ERROR("unexpected evcpe_method_type: %d", parser->msg->method_type);
			goto syntax_error;
		}

		if (len > 32) {
			ERROR("Sizeof error : %s:%d", text, len);
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(key_holder, text, len);
	} else if (elm_type == 	NODE_FileType) {
		download = parser->msg->data;
		if (len > sizeof(download->file_type_str)) {
			rc = EOVERFLOW;
			goto finally;
		}
		if ((download->file_type = evcpe_file_type_from_string(text, len))
				== EVCPE_FILE_TYPE_UNKNOWN) {
			ERROR("unexpected file type: %.*s", len, text);
			goto syntax_error;
		}
		strncpy(download->file_type_str, text, len);
	} else if(elm_type == NODE_URL) {
		char url_str[257];
		if (len > 256) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(url_str, text, len);
		download = parser->msg->data;
		download->url = evcpe_url_new();
		evcpe_url_from_str(download->url, url_str);
	} else if (elm_type == NODE_SuccessURL) {
		char url_str[257];
		if (len > 256) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(url_str, text, len);
		download = parser->msg->data;
		download->success_url = evcpe_url_new();
		evcpe_url_from_str(download->success_url, url_str);
	} else if (elm_type == NODE_FailureURL) {
		char url_str[257];
		if (len > 256) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(url_str, text, len);
		download = parser->msg->data;
		download->failure_url = evcpe_url_new();
		evcpe_url_from_str(download->failure_url, url_str);
	} else if(elm_type == NODE_Username) {
		download = parser->msg->data;
		if (len > sizeof(download->username)) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(download->username, text, len);
	} else if(elm_type == NODE_Passwrod) {
		download = parser->msg->data;
		download = parser->msg->data;
		if (len > sizeof(download->password)) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(download->password, text, len);
	} else if (elm_type == NODE_FileSize) {
		download = parser->msg->data;
		if ((rc = evcpe_atol(text, len, &val)) || val < 0)
			goto syntax_error;
		download->file_size = val;
	} else if (elm_type == NODE_TargetFileName) {
		download = parser->msg->data;
		if (len > sizeof(download->target_filename)) {
			rc = EOVERFLOW;
			goto finally;
		}
		strncpy(download->target_filename, text, len);
	} else if (elm_type == NODE_DelaySeconds) {
		download = parser->msg->data;
		if ((rc = evcpe_atol(text, len, &val)) || val < 0)
			goto syntax_error;
		download->delay = val;
	} else if (len > 0) {
		ERROR("unexpected element: %.*s", elm->len, elm->name);
		goto syntax_error;
	}
	rc = 0;

finally:
	return rc;

syntax_error:
	ERROR("syntax error");
	return EPROTO;
}

static
int evcpe_msg_xml_attr_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc = 0;
	evcpe_msg_parser *parser = data;
	evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);

	TRACE("attribute: %.*s => %.*s (namespace: %.*s)",
			name_len, name, value_len, value, nslen, ns);

	if (nslen && !evcpe_strncmp("xmlns", ns, nslen)) {
		if ((rc = evcpe_xmlns_table_add(&parser->xmlns, name, name_len, value,
				value_len)))
			goto finally;
		if (!evcpe_strcmp(parent->ns, parent->nslen, name, name_len))
			parent->ns_declared = 1;
	}

finally:
	return rc;
}

int evcpe_msg_from_xml(evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc = 0;
	evcpe_msg_parser parser;
	evcpe_xml_element *elm;

	DEBUG("unmarshaling SOAP message");

	if (!evbuffer_get_length(buffer)) return 0;

	if (!_tree_ready) _msg_tree_init();

	parser.msg = msg;
	parser.xml.data = &parser;
	parser.xml.xmlstart = (const char *)evbuffer_pullup(buffer, -1);
	parser.xml.xmlsize = evbuffer_get_length(buffer);
	parser.xml.starteltfunc = evcpe_msg_xml_elm_begin_cb;
	parser.xml.endeltfunc = evcpe_msg_xml_elm_end_cb;
	parser.xml.datafunc = evcpe_msg_xml_data_cb;
	parser.xml.attfunc = evcpe_msg_xml_attr_cb;
	RB_INIT(&parser.xmlns);
	SLIST_INIT(&parser.stack);
	if ((rc = parsexml(&parser.xml))) {
		ERROR("failed to parse SOAP message: %d", rc);
	}
	while((elm = evcpe_xml_stack_pop(&parser.stack))) {
		ERROR("pending stack: %.*s", elm->len, elm->name);
		free(elm);
	}
	evcpe_xmlns_table_clear(&parser.xmlns);

	return rc;
}
