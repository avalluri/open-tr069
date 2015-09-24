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

#ifndef EVCPE_GET_PARAM_NAMES_H_
#define EVCPE_GET_PARAM_NAMES_H_

#include "data.h"

struct evcpe_get_param_names {
	char parameter_path[257];
	int next_level;
};

struct evcpe_get_param_names *evcpe_get_param_names_new(void);

void evcpe_get_param_names_free(struct evcpe_get_param_names *method);

struct evcpe_get_param_names_response {
	struct evcpe_param_info_list parameter_list;
};

struct evcpe_get_param_names_response *evcpe_get_param_names_response_new(void);

void evcpe_get_param_names_response_free(
		struct evcpe_get_param_names_response *method);

int evcpe_get_param_names_response_to_xml(
		struct evcpe_get_param_names_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_GET_PARAM_NAMES_H_ */
