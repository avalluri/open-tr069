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

#include "inform.h"

struct evcpe_inform *evcpe_inform_new(void)
{
	struct evcpe_inform *inform;

	DEBUG("constructing evcpe_inform");

	if (!(inform = calloc(1, sizeof(struct evcpe_inform)))) {
		ERROR("failed to calloc evcpe_inform");
		return NULL;
	}
	inform->max_envelopes = 1;
	evcpe_event_list_init(&inform->event);
	evcpe_param_value_list_init(&inform->parameter_list);

	return inform;
}

void evcpe_inform_free(struct evcpe_inform *inform)
{
	if (!inform) return;

	DEBUG("destructing evcpe_inform");

	evcpe_event_list_clear(&inform->event);
	evcpe_param_value_list_clear(&inform->parameter_list);
	free(inform);
}

struct evcpe_inform_response *evcpe_inform_response_new(void)
{
	struct evcpe_inform_response *method;

	DEBUG("constructing evcpe_inform_response");

	if (!(method = calloc(1, sizeof(struct evcpe_inform_response)))) {
		ERROR("failed to calloc evcpe_inform_response");
		return NULL;
	}
	return method;
}

void evcpe_inform_response_free(struct evcpe_inform_response *inform)
{
	if (!inform) return;
	DEBUG("destructing evcpe_inform_response");
	free(inform);
}

int evcpe_inform_to_xml(struct evcpe_inform *method, struct evbuffer *buffer)
{
	int rc;

	DEBUG("marshaling inform");

	if ((rc = evcpe_device_id_to_xml(&method->device_id, "DeviceId", buffer)))
		goto finally;
	if ((rc = evcpe_event_list_to_xml(&method->event, "Event", buffer)))
		goto finally;
	if ((rc = evcpe_xml_add_int(buffer,
			"MaxEnvelopes", method->max_envelopes)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer,
			"CurrentTime", method->current_time)))
		goto finally;
	if ((rc = evcpe_xml_add_int(buffer,
			"RetryCount", method->retry_count)))
		goto finally;
	if ((rc = evcpe_param_value_list_to_xml(&method->parameter_list,
			"ParameterList", buffer)))
		goto finally;

finally:
	return rc;
}
