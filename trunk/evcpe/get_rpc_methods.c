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

#include "get_rpc_methods.h"

struct evcpe_get_rpc_methods *evcpe_get_rpc_methods_new(void)
{
	struct evcpe_get_rpc_methods *method;

	evcpe_debug(__func__, "constructing evcpe_get_rpc_methods");

	if (!(method = calloc(1, sizeof(struct evcpe_get_rpc_methods)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_rpc_methods");
		return NULL;
	}
	return method;
}

void evcpe_get_rpc_methods_free(struct evcpe_get_rpc_methods *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_rpc_methods");
	free(method);
}

struct evcpe_get_rpc_methods_response *evcpe_get_rpc_methods_response_new(void)
{
	struct evcpe_get_rpc_methods_response *method;
	evcpe_debug(__func__, "constructing evcpe_get_rpc_methods_response");
	if (!(method = calloc(1, sizeof(struct evcpe_get_rpc_methods_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_get_rpc_methods_response");
		return NULL;
	}
	evcpe_method_list_init(&method->method_list);
	return method;
}

void evcpe_get_rpc_methods_response_free(
		struct evcpe_get_rpc_methods_response *method)
{
	if (!method) return;
	evcpe_debug(__func__, "destructing evcpe_get_rpc_methods_response");
	evcpe_method_list_clear(&method->method_list);
	free(method);
}
