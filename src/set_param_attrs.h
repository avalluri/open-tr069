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

#ifndef EVCPE_SET_PARAM_ATTRS_H_
#define EVCPE_SET_PARAM_ATTRS_H_

#include <event.h>

#include "data.h"

typedef struct _evcpe_set_param_attrs {
	tqueue* parameter_list;
} evcpe_set_param_attrs;

typedef struct _evcpe_set_param_attrs_response {
	int status;
} evcpe_set_param_attrs_response;

evcpe_set_param_attrs *evcpe_set_param_attrs_new(void);

void evcpe_set_param_attrs_free(evcpe_set_param_attrs *method);

evcpe_set_param_attrs_response* evcpe_set_param_attrs_response_new();

void evcpe_set_param_attrs_response_free(evcpe_set_param_attrs_response* resp);

int evcpe_set_param_attrs_response_to_xml(evcpe_set_param_attrs_response* resp,
		struct evbuffer* buffer);

#endif /* EVCPE_SET_PARAM_ATTRS_H_ */
