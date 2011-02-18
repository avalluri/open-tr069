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

#ifndef EVCPE_CPE_H_
#define EVCPE_CPE_H_

#include "handler.h"
#include "dns_cache.h"
#include "method.h"
#include "xml.h"
#include "repo.h"
#include "url.h"
#include "session.h"
#include "persister.h"

#include <evcpe.h>

typedef int (*evcpe_request_hook_cb)(enum evcpe_method_type type, void *data);

enum evcpe_auth_type {
	EVCPE_AUTH_NONE,
	EVCPE_AUTH_BASIC,
	EVCPE_AUTH_DIGEST
};

struct evcpe {
	struct event_base *evbase;

	evcpe_request_cb cb;
	evcpe_error_cb error_cb;
	void *cbarg;

	struct evcpe_dns_cache dns_cache;

	struct evcpe_repo *repo;
//	struct evcpe_persister *persist;

//	struct evcpe_device_id id;
//	struct evcpe_event_list events;

	enum evcpe_auth_type acs_auth;
	struct evcpe_url *acs_url;
	const char *acs_username;
	const char *acs_password;
	unsigned int acs_timeout;

	struct evcpe_url *proxy_url;
	const char *proxy_username;
	const char *proxy_password;

	struct evcpe_url *creq_url;
	const char *creq_username;
	const char *creq_password;
	unsigned int creq_interval;
	time_t creq_last;

	struct evhttp *http;

	struct evcpe_session *session;

	unsigned int retry_count;
	struct event retry_ev;
	struct timeval retry_tv;

	struct event periodic_ev;
	struct timeval periodic_tv;
};

struct evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb cb, evcpe_error_cb error_cb, void *cbarg);

void evcpe_free(struct evcpe *cpe);

int evcpe_set(struct evcpe *cpe, struct evcpe_repo *repo);

int evcpe_bind_http(struct evcpe *cpe, const char *address, u_short port,
		const char *prefix);

int evcpe_is_attached(struct evcpe *cpe);

int evcpe_make_request(struct evcpe *cpe, const char *acs_url,
		struct evcpe_request *req);

int evcpe_set_acs(struct evcpe *cpe, const char *address, u_short port,
		const char *uri);

int evcpe_set_auth(struct evcpe *cpe, enum evcpe_auth_type type,
		const char *username, const char *password);

int evcpe_start(struct evcpe *cpe);

//int evcpe_add_event(struct evcpe *cpe,
//		const char *event_code, const char *command_key);
//
//int evcpe_remove_event(struct evcpe *cpe,
//		const char *event_code);

#endif /* EVCPE_CPE_H_ */
