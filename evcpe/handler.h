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

#ifndef EVCPE_HANDLER_H_
#define EVCPE_HANDLER_H_

#include <sys/tree.h>

#include "request.h"
#include "error.h"

struct evcpe;

typedef void (*evcpe_request_cb)(struct evcpe_request *req, void *cbarg);

typedef void (*evcpe_error_cb)(struct evcpe *cpe,
		enum evcpe_error_type type, int code, const char *reason, void *cbarg);

struct evcpe_handler {
//	enum evcpe_request_type type;
	evcpe_request_cb cb;
	void *cbarg;
	RB_ENTRY(evcpe_handler) entry;
};

RB_HEAD(evcpe_handlers, evcpe_handler);

int evcpe_handler_cmp(struct evcpe_handler *a, struct evcpe_handler *b);

RB_PROTOTYPE(evcpe_handlers, evcpe_handler, entry, evcpe_handler_cmp);

#endif /* EVCPE_HANDLER_H_ */
