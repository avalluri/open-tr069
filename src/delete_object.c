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

#include "delete_object.h"

struct evcpe_delete_object *evcpe_delete_object_new(void)
{
	struct evcpe_delete_object *method;

	DEBUG("constructing evcpe_delete_object");

	if (!(method = calloc(1, sizeof(struct evcpe_delete_object)))) {
		ERROR("failed to calloc evcpe_add_obejct");
		return NULL;
	}
	return method;
}

void evcpe_delete_object_free(struct evcpe_delete_object *method)
{
	if (!method) return;
	DEBUG("destructing evcpe_delete_object");
	free(method);
}

struct evcpe_delete_object_response *evcpe_delete_object_response_new(void)
{
	return (struct evcpe_delete_object_response *) calloc(1,
			sizeof(struct evcpe_delete_object_response));
}

void evcpe_delete_object_response_free(struct evcpe_delete_object_response *resp)
{
	if (resp) free(resp);
}

int evcpe_delete_object_response_to_xml(struct evcpe_delete_object_response *resp,
		struct evbuffer *buffer)
{
	return evcpe_xml_add_xsd_int(buffer, "Status", resp->status);
}
