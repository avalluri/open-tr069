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

#ifndef EVCPE_INFORM_H_
#define EVCPE_INFORM_H_

#include <time.h>

#include "data.h"

struct evcpe_inform {
	struct evcpe_device_id device_id;
	struct evcpe_event_list event;
	unsigned int max_envelopes;
	char current_time[26];
	unsigned int retry_count;
	struct evcpe_param_value_list parameter_list;
};

struct evcpe_inform *evcpe_inform_new(void);

void evcpe_inform_free(struct evcpe_inform *inform);

int evcpe_inform_add_event(struct evcpe_inform *inform,
		const char *event_code, const char *command_key);

struct evcpe_inform_response {
	unsigned int max_envelopes;
};

struct evcpe_inform_response *evcpe_inform_response_new(void);

void evcpe_inform_response_free(struct evcpe_inform_response *inform);

#endif /* EVCPE_INFORM_H_ */
