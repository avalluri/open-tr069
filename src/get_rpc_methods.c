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
#include "util.h"
#include "evcpe-config.h"

#include "get_rpc_methods.h"

evcpe_get_rpc_methods *evcpe_get_rpc_methods_new(void)
{
	evcpe_get_rpc_methods *method;

	DEBUG("constructing evcpe_get_rpc_methods");

	if (!(method = calloc(1, sizeof(evcpe_get_rpc_methods)))) {
		ERROR("failed to calloc evcpe_get_rpc_methods");
		return NULL;
	}
	return method;
}

void evcpe_get_rpc_methods_free(evcpe_get_rpc_methods *method)
{
	if (!method) return;
	DEBUG("destructing evcpe_get_rpc_methods");
	free(method);
}

evcpe_get_rpc_methods_response *evcpe_get_rpc_methods_response_new(void)
{
	evcpe_get_rpc_methods_response *response;
	DEBUG("constructing evcpe_get_rpc_methods_response");
	if (!(response = calloc(1, sizeof(evcpe_get_rpc_methods_response)))) {
		ERROR("failed to calloc evcpe_get_rpc_methods_response");
		return NULL;
	}
	response->method_list = tqueue_new(NULL, NULL);

	return response;
}

void evcpe_get_rpc_methods_response_free(
		evcpe_get_rpc_methods_response *response)
{
	if (!response) return;
	DEBUG("destructing evcpe_get_rpc_methods_response");
	tqueue_free(response->method_list);
	free(response);
}


int evcpe_get_rpc_methods_response_to_xml(
		evcpe_get_rpc_methods_response *response, struct evbuffer *buffer)
{

	int rc = 0, i = 0;
	const char* node_name = "MethodList";
	tqueue_element* node = NULL;

	DEBUG("marshaling evcpe_get_rpc_methods_response");

	rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\"xsd:string[%zu]\">\n",
			node_name, tqueue_size(response->method_list));

	if (rc) return rc;

	TQUEUE_FOREACH(node, response->method_list) {
		if ((rc = evcpe_xml_add_string(buffer, "string",
				evcpe_method_type_to_str((evcpe_method_type_t)node->data))))
			return rc;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node_name)))
		return rc;

	return 0;
}

