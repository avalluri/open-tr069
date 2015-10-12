/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of open-tr069
 *
 * Copyright (C) 2013-2015 Intel Corporation.
 *
 * Contact: Amarnath Valluri <amarnath.valluri@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <errno.h>
#include <stdlib.h>

#include "xml.h"

#include "download.h"

evcpe_download* evcpe_download_new()
{
	evcpe_download* req = calloc(1, sizeof(*req));

	return req;
}

void evcpe_download_free(evcpe_download* req)
{
	if (req) free(req);
}

evcpe_download_response* evcpe_download_response_new()
{
	evcpe_download_response* resp = calloc(1, sizeof(*resp));

	return resp;
}

void evcpe_download_response_free(evcpe_download_response* resp)
{
	if (resp) free(resp);
}

int evcpe_download_response_to_xml(evcpe_download_response* resp,
		struct evbuffer* buffer)
{
	int rc = 0;
	if (!resp) return EINVAL;

	if ((rc = evcpe_xml_add_xsd_int(buffer, "Status", resp->status)) ||
	    (rc = evcpe_xml_add_datetime(buffer, "StartTime", resp->start_time)) ||
		(rc = evcpe_xml_add_datetime(buffer, "CompleteTime", resp->end_time))) {
		return rc;
	}

	return 0;
}
