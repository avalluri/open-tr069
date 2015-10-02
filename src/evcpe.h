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

#ifndef EVCPE_H_
#define EVCPE_H_

#include <sys/types.h>
#include <stdio.h>
#include <evhttp.h>

#include "handler.h"
#include "dns_cache.h"
#include "method.h"
#include "xml.h"
#include "repo.h"
#include "url.h"
#include "session.h"
#include "persister.h"

#define EVCPE_IANA_CWMP_PORT 7547

typedef int (*evcpe_request_hook_cb_t)(evcpe_method_type_t type, void *data);

typedef enum _evcpe_auth_type {
	EVCPE_AUTH_NONE,
	EVCPE_AUTH_BASIC,
	EVCPE_AUTH_DIGEST
} evcpe_auth_type_t;

typedef struct _evcpe {
	struct event_base *evbase;

	evcpe_request_cb_t cb;
	evcpe_error_cb_t error_cb;
	void *cbarg;

	struct evdns_base *dnsbase;
	evcpe_dns_cache dns_cache;


	evcpe_repo *repo;
//	struct evcpe_persister *persist;

//	struct evcpe_device_id id;
//	struct evcpe_event_list events;

	evcpe_auth_type_t acs_auth;
	evcpe_url *acs_url;
	const char *acs_username;
	const char *acs_password;
	unsigned int acs_timeout;

	evcpe_url *proxy_url;
	const char *proxy_username;
	const char *proxy_password;

	evcpe_url *creq_url;
	const char *creq_username;
	const char *creq_password;
	unsigned int creq_interval;
	time_t creq_last;

	struct evhttp *http;

	evcpe_session *session;

	unsigned int retry_count;
	struct event retry_ev;
	struct timeval retry_tv;

	struct event periodic_ev;
	struct timeval periodic_tv;
} evcpe;

evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb_t cb, evcpe_error_cb_t error_cb, void *cbarg);

void evcpe_free(evcpe *cpe);

int evcpe_set(evcpe *cpe, evcpe_repo *repo);

int evcpe_bind_http(evcpe *cpe, const char *address, u_short port,
		const char *prefix);

int evcpe_is_attached(evcpe *cpe);

int evcpe_make_request(evcpe *cpe, const char *acs_url, evcpe_request *req);

int evcpe_set_acs(evcpe *cpe, const char *address, u_short port,
		const char *uri);

int evcpe_set_auth(evcpe *cpe, evcpe_auth_type_t type,
		const char *username, const char *password);

int evcpe_start(evcpe *cpe, int bootstrap);

//int evcpe_add_event(evcpe *cpe,
//		const char *event_code, const char *command_key);
//
//int evcpe_remove_event(evcpe *cpe,
//		const char *event_code);

#endif /* EVCPE_H_ */
