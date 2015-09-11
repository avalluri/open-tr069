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

#include "log.h"
#include "data_xml.h"

#include "set_param_values_xml.h"

int evcpe_set_param_values_response_to_xml(
		struct evcpe_set_param_values_response *method,
		struct evbuffer *buffer)
{
	int rc;
	DEBUG("marshaling evcpe_set_param_values_response");
	if ((rc = evcpe_xml_add_xsd_int(buffer, "Status",
			method->status)))
		goto finally;
finally:
	return rc;
}
