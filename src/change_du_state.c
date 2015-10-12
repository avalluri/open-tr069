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

#include <stdio.h>

#include "change_du_state.h"

evcpe_change_du_state* evcpe_change_du_state_new()
{
	evcpe_change_du_state* req = calloc(1, sizeof(*req));
	if (!req) return NULL;

	req->operations = tqueue_new(NULL, free);

	return req;
}

void evcpe_change_du_state_free(evcpe_change_du_state* req)
{
	if (req) {
		tqueue_free(req->operations);
		free(req);
	}
}
