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

#include "get_param_attrs.h"

evcpe_get_param_attrs *evcpe_get_param_attrs_new(void)
{
	evcpe_get_param_attrs *method;
	DEBUG("constructing evcpe_get_param_attrs");
	if (!(method = calloc(1, sizeof(evcpe_get_param_attrs)))) {
		ERROR("failed to calloc evcpe_get_param_attrs");
		return NULL;
	}
	method->parameter_names = tqueue_new(NULL, free);
	return method;
}

void evcpe_get_param_attrs_free(evcpe_get_param_attrs *method)
{
	if (!method) return;
	DEBUG("destructing evcpe_get_param_attrs");
	tqueue_free(method->parameter_names);
	free(method);
}

evcpe_get_param_attrs_response *evcpe_get_param_attrs_response_new(void)
{
	evcpe_get_param_attrs_response *method;
	DEBUG("constructing evcpe_get_param_attrs_response");
	if (!(method = calloc(1, sizeof(evcpe_get_param_attrs_response)))) {
		ERROR("failed to calloc evcpe_get_param_attrs");
		return NULL;
	}
	method->parameter_list = evcpe_param_attr_list_new();
	return method;
}

void evcpe_get_param_attrs_response_free(evcpe_get_param_attrs_response *method)
{
	if (!method) return;
	DEBUG("destructing evcpe_get_param_attrs_response");
	tqueue_free(method->parameter_list);
	free(method);
}
