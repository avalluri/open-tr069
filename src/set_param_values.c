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

#include "set_param_values.h"

evcpe_set_param_values *evcpe_set_param_values_new(void)
{
	evcpe_set_param_values *method;

	if (!(method = calloc(1, sizeof(evcpe_set_param_values)))) {
		ERROR("failed to calloc evcpe_set_param_values");
		return NULL;
	}
	method->parameter_list = evcpe_param_value_list_new();
	return method;
}

void evcpe_set_param_values_free(evcpe_set_param_values *method)
{
	if (!method) return;

	tqueue_free(method->parameter_list);

	free(method);
}

evcpe_set_param_values_response *evcpe_set_param_values_response_new(void)
{
	evcpe_set_param_values_response *response;

	if (!(response = calloc(1, sizeof(evcpe_set_param_values_response)))) {
		ERROR("failed to calloc evcpe_set_param_values_response");
		return NULL;
	}
	return response;
}

void evcpe_set_param_values_response_free(
		evcpe_set_param_values_response *response)
{
	if (response) free(response);
}

int evcpe_set_param_values_response_to_xml(
		evcpe_set_param_values_response *method,
		struct evbuffer *buffer)
{
	DEBUG("marshaling evcpe_set_param_values_response");
	return evcpe_xml_add_xsd_int(buffer, "Status", method->status);
}

