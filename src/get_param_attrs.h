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

#ifndef EVCPE_GET_PARAM_ATTRS_H_
#define EVCPE_GET_PARAM_ATTRS_H_

#include "data.h"

typedef struct _evcpe_get_param_attrs {
	tqueue* parameter_names;
} evcpe_get_param_attrs;

evcpe_get_param_attrs *evcpe_get_param_attrs_new(void);

void evcpe_get_param_attrs_free(evcpe_get_param_attrs *method);

typedef struct _evcpe_get_param_attrs_response {
	tqueue* parameter_list;
} evcpe_get_param_attrs_response;

evcpe_get_param_attrs_response *evcpe_get_param_attrs_response_new(void);

void evcpe_get_param_attrs_response_free(evcpe_get_param_attrs_response *method);

#endif /* EVCPE_GET_PARAM_ATTRS_H_ */
