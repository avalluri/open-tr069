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

#ifndef EVCPE_SESSION_H_
#define EVCPE_SESSION_H_

#include <evhttp.h>

#include "msg.h"
#include "url.h"
#include "cookie.h"
#include "dns_cache.h"

typedef struct _evcpe_session evcpe_session;

typedef void (*evcpe_session_cb_t)(evcpe_session *session,
		evcpe_msg_type_t type, evcpe_method_type_t method_type,
		void *request, void *response, void *cbarg);

typedef void (*evcpe_session_close_cb_t)(evcpe_session *session,
		int code, void *cbarg);

struct _evcpe_session {
	struct event_base *evbase;
	struct evhttp_connection *conn;
	evcpe_cookies cookies;
	int hold_requests;
	int no_more_requests;
	const char *address;
	u_short port;
	unsigned int timeout;
	evcpe_url *acs;
	tqueue* req_in;
	tqueue* req_out;
	tqueue* req_pending;
	tqueue* res_pending;
	evcpe_session_cb_t cb;
	void *cbarg;
	evcpe_session_close_cb_t close_cb;
	void *close_cbarg;
};

evcpe_session *evcpe_session_new(struct evhttp_connection *conn,
		evcpe_url *acs, evcpe_session_cb_t cb, void *cbarg);

void evcpe_session_free(evcpe_session *session);

void evcpe_session_close(evcpe_session *session, int code);

void evcpe_session_set_close_cb(evcpe_session *session,
		evcpe_session_close_cb_t close_cb, void *cbarg);

int evcpe_session_send(evcpe_session *session, evcpe_msg *msg);

int evcpe_session_start(evcpe_session *session);

#endif /* EVCPE_SESSION_H_ */
