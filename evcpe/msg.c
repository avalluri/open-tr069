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

#include <stdlib.h>

#include "log.h"
#include "fault.h"
#include "inform.h"
#include "get_rpc_methods.h"
#include "get_param_names.h"
#include "get_param_attrs.h"
#include "set_param_attrs.h"
#include "get_param_values.h"
#include "set_param_values.h"
#include "add_object.h"
#include "delete_object.h"

#include "msg.h"

const char *evcpe_msg_type_to_str(enum evcpe_msg_type type)
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

void evcpe_msg_queue_clear(struct evcpe_msg_queue *queue)
{
	struct evcpe_msg *msg;
	evcpe_debug(__func__, "clearing evcpe_msg_queue");
	while ((msg = TAILQ_FIRST(queue))) {
		TAILQ_REMOVE(queue, msg, entry);
		evcpe_msg_free(msg);
	}
}

struct evcpe_msg *evcpe_msg_new(void)
{
	struct evcpe_msg *msg;

	evcpe_debug(__func__, "constructing evcpe_msg");

	if (!(msg = calloc(1, sizeof(struct evcpe_msg)))) {
		evcpe_error(__func__, "failed to calloc evcpe_msg");
		return NULL;
	}
	msg->major = 1;
	msg->minor = 0;
	return msg;
}

void evcpe_msg_free(struct evcpe_msg *msg)
{
	if (!msg) return;

	evcpe_debug(__func__, "destructing evcpe_msg");

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
			case EVCPE_INFORM:
				evcpe_inform_free(msg->data);
				break;
			default:
				evcpe_error(__func__, "unexpected request type: %d",
						msg->method_type);
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
				evcpe_error(__func__, "unexpected response type: %d",
						msg->method_type);
			}
			break;
		case EVCPE_MSG_FAULT:
			evcpe_fault_free(msg->data);
			break;
		default:
			evcpe_error(__func__, "unexpected message type: %d",
					msg->type);
		}
	}
	if (msg->session) free(msg->session);
	free(msg);
}
