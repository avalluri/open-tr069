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
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <evdns.h>

#include "log.h"
#include "util.h"
#include "msg.h"
#include "data.h"
#include "fault.h"
#include "inform.h"
#include "get_rpc_methods.h"
#include "get_param_names.h"
#include "get_param_values.h"
#include "get_param_attrs.h"
#include "add_object.h"
#include "set_param_values.h"

#include "cpe.h"

static void evcpe_send_error(struct evcpe *cpe, enum evcpe_error_type type,
		int code, const char *reason);

static void evcpe_creq_cb(struct evhttp_request *req, void *arg);

static void evcpe_session_message_cb(struct evcpe_session *session,
		enum evcpe_msg_type type, enum evcpe_method_type method_type,
				void *request, void *response, void *arg);

static void evcpe_session_terminate_cb(struct evcpe_session *session,
		int code, void *arg);

static void evcpe_dns_cb(int result, char type, int count, int ttl,
	    void *addresses, void *arg);

static inline void evcpe_dns_timer_cb(int fd, short event, void *arg);

static int evcpe_dns_entry_resolve(struct evcpe_dns_entry *entry,
		const char *hostname);

static int evcpe_dns_add(struct evcpe *cpe, const char *hostname);

static inline int evcpe_handle_request(struct evcpe *cpe,
		struct evcpe_session *session,
		enum evcpe_method_type method_type, void *request);

static inline int evcpe_handle_response(struct evcpe *cpe,
		enum evcpe_method_type method_type, void *request, void *response);

static inline int evcpe_handle_get_rpc_methods(struct evcpe *cpe,
		struct evcpe_get_rpc_methods *req, struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_names(struct evcpe *cpe,
		struct evcpe_get_param_names *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_values(struct evcpe *cpe,
		struct evcpe_get_param_values *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_attrs(struct evcpe *cpe,
		struct evcpe_get_param_attrs *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_add_object(struct evcpe *cpe,
		struct evcpe_add_object *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_set_param_values(struct evcpe *cpe,
		struct evcpe_set_param_values *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_inform_response(struct evcpe *cpe,
		struct evcpe_inform *req, struct evcpe_inform_response *resp);

static int evcpe_retry_session(struct evcpe *cpe);

static void evcpe_start_session_cb(int fd, short event, void *arg);

static int evcpe_start_session(struct evcpe *cpe);

struct evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb cb, evcpe_error_cb error_cb, void *cbarg)
{
	struct evcpe *cpe;

	evcpe_debug(__func__, "constructing evcpe");

	if ((cpe = calloc(1, sizeof(struct evcpe))) == NULL) {
		evcpe_error(__func__, "failed to calloc evcpe");
		return NULL;
	}
	RB_INIT(&cpe->dns_cache);
	cpe->evbase = evbase;
	cpe->cb = cb;
	cpe->error_cb = error_cb;
	cpe->cbarg = cbarg;

	return cpe;
}

void evcpe_free(struct evcpe *cpe)
{
	if (cpe == NULL) return;

	evcpe_debug(__func__, "destructing evcpe");

	if (event_initialized(&cpe->retry_ev) &&
			evtimer_pending(&cpe->retry_ev, NULL)) {
		event_del(&cpe->retry_ev);
	}
	if (event_initialized(&cpe->periodic_ev) &&
			evtimer_pending(&cpe->periodic_ev, NULL)) {
		event_del(&cpe->periodic_ev);
	}
	if (cpe->session) evcpe_session_free(cpe->session);
	if (cpe->http) evhttp_free(cpe->http);
	if (cpe->acs_url) evcpe_url_free(cpe->acs_url);
	if (cpe->proxy_url) evcpe_url_free(cpe->proxy_url);
	if (cpe->creq_url) evcpe_url_free(cpe->creq_url);

	evcpe_dns_cache_clear(&cpe->dns_cache);
	free(cpe);
}

int evcpe_set(struct evcpe *cpe,
		struct evcpe_repo *repo)
{
	int rc, number;
	const char *value;
	unsigned int len;

	if ((rc = evcpe_repo_get(repo,
			".ManagementServer.Authentication", &value, &len))) {
		evcpe_error(__func__, "failed to get ACS authentication");
		goto finally;
	}
	if (!strcmp("NONE", value))
		cpe->acs_auth = EVCPE_AUTH_NONE;
	else if (!strcmp("BASIC", value))
		cpe->acs_auth = EVCPE_AUTH_NONE;
	else if (!strcmp("DIGEST", value))
		cpe->acs_auth = EVCPE_AUTH_NONE;
	else {
		evcpe_error(__func__, "invalid authentication value: %s", value);
		rc = EINVAL;
		goto finally;
	}

	if ((rc = evcpe_repo_get(repo,
			".ManagementServer.URL", &value, &len))) {
		evcpe_error(__func__, "failed to get ACS URL");
		goto finally;
	}
	if (!(cpe->acs_url = evcpe_url_new())) {
		evcpe_error(__func__, "failed to create evcpe_url");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_url_from_str(cpe->acs_url, value))) {
		evcpe_error(__func__, "failed to parse ACS URL: %s", value);
		goto finally;
	}
	if ((rc = evcpe_dns_add(cpe, cpe->acs_url->host))) {
		evcpe_error(__func__, "failed to resolve ACS hostname");
		goto finally;
	}
	cpe->acs_username = evcpe_repo_find(repo,
			".ManagementServer.Username");
	cpe->acs_password = evcpe_repo_find(repo,
			".ManagementServer.Password");
	if ((value = evcpe_repo_find(repo,
			".ManagementServer.Timeout"))) {
		if ((number = atoi(value)) < 0) {
			evcpe_error(__func__, "invalid ACS timeout: %d", number);
			rc = EINVAL;
			goto finally;
		}
		cpe->acs_timeout = number;
	} else {
		cpe->acs_timeout = EVCPE_ACS_TIMEOUT;
	}

	if ((value = evcpe_repo_find(repo,
			".ManagementServer.ProxyURL"))) {
		if (!(cpe->proxy_url = evcpe_url_new())) {
			evcpe_error(__func__, "failed to create evcpe_url");
			rc = ENOMEM;
			goto finally;
		}
		if ((rc = evcpe_url_from_str(cpe->proxy_url, value))) {
			evcpe_error(__func__, "failed to parse proxy URL: %s", value);
			goto finally;
		}
		if ((rc = evcpe_dns_add(cpe, cpe->proxy_url->host))) {
			evcpe_error(__func__, "failed to resolve HTTP proxy hostname");
			goto finally;
		}
		cpe->proxy_username = evcpe_repo_find(repo,
				".ManagementServer.ProxyUsername");
		cpe->proxy_password = evcpe_repo_find(repo,
				".ManagementServer.ProxyPassword");
	}

	if ((value = evcpe_repo_find(repo,
			".ManagementServer.ConnectionRequestURL"))) {
		if (!(cpe->creq_url = evcpe_url_new())) {
			evcpe_error(__func__, "failed to create evcpe_url");
			rc = ENOMEM;
			goto finally;
		}
		if ((rc = evcpe_url_from_str(cpe->creq_url, value))) {
			evcpe_error(__func__, "failed to parse ACS URL");
			goto finally;
		}
		cpe->creq_username = evcpe_repo_find(repo,
				".ManagementServer.ConnectionRequestUsername");
		cpe->creq_password = evcpe_repo_find(repo,
				".ManagementServer.ConnectionRequestPassword");
		if ((value = evcpe_repo_find(repo,
				".ManagementServer.ConnectionRequestInterval"))) {
			if ((number = atoi(value)) < 0) {
				evcpe_error(__func__, "invalid connection request interval: %d",
						number);
				rc = EINVAL;
				goto finally;
			}
			cpe->creq_interval = number;
		} else {
			cpe->creq_interval = EVCPE_CREQ_INTERVAL;
		}
	}

	if ((value = evcpe_repo_find(repo,
			".ManagementServer.PeriodicInformEnable")) &&
			!strcmp("1", value)) {
		if (!(value = evcpe_repo_find(repo,
				".ManagementServer.PeriodicInformInterval"))) {
			evcpe_error(__func__, "periodic inform interval was not set");
			rc = EINVAL;
			goto finally;
		}
		if ((number = atoi(value)) < 0) {
			evcpe_error(__func__, "invalid periodic inform interval: %d",
					number);
			rc = EINVAL;
			goto finally;
		}
		evtimer_set(&cpe->periodic_ev, evcpe_start_session_cb, cpe);
		if ((rc = event_base_set(cpe->evbase, &cpe->periodic_ev))) {
			evcpe_error(__func__, "failed to set event base");
			goto finally;
		}
		evutil_timerclear(&cpe->periodic_tv);
		cpe->periodic_tv.tv_sec = number;
		evcpe_info(__func__, "scheduling periodic inform in %ld second(s)",
				cpe->periodic_tv.tv_sec);
		if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv))) {
			evcpe_error(__func__, "failed to schedule periodic inform");
			goto finally;
		}
	}
	cpe->repo = repo;

finally:
	return rc;
}

void evcpe_creq_cb(struct evhttp_request *req, void *arg)
{
	time_t curtime;
	struct evcpe *cpe = arg;

	if (req->type != EVHTTP_REQ_GET || (cpe->creq_url->uri &&
			strncmp(cpe->creq_url->uri,
			evhttp_request_uri(req), strlen(cpe->creq_url->uri)))) {
		evhttp_send_reply(req, 404, "Not Found", NULL);
		return;
	} else {
		curtime = time(NULL);
		if (cpe->creq_last && difftime(curtime, cpe->creq_last) <
				cpe->creq_interval) {
			evhttp_send_reply(req, 503, "Service Unavailable", NULL);
			return;
		} else {
			if (cpe->session) {
				evhttp_send_reply(req, 503, "Session in Progress", NULL);
				return;
			}
			// TODO: handle auth
			if (evcpe_repo_add_event(cpe->repo, "6 CONNECTION REQUEST", "")) {
				evcpe_error(__func__, "failed to add connection request event");
				evhttp_send_reply(req, 501, "Internal Server Error", NULL);
				return;
			}
			if (evcpe_start_session(cpe)) {
				evcpe_error(__func__, "failed to start session");
				evhttp_send_reply(req, 501, "Internal Server Error", NULL);
				return;
			} else {
				cpe->creq_last = curtime;
				evhttp_send_reply(req, 200, "OK", NULL);
				return;
			}
		}
	}
}

int evcpe_bind(struct evcpe *cpe)
{
	int rc;
	struct evhttp *http;

	evcpe_info(__func__, "binding %s on port: %d",
			cpe->creq_url->protocol, cpe->creq_url->port);

	// TODO: SSL

	if (!(http = evhttp_new(cpe->evbase))) {
		evcpe_error(__func__, "failed to create evhttp");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evhttp_bind_socket(http, "0.0.0.0", cpe->creq_url->port)) != 0) {
		evcpe_error(__func__, "failed to bind evhttp on port:%d",
				cpe->creq_url->port);
		evhttp_free(http);
		goto finally;
	}

	if (cpe->http) evhttp_free(cpe->http);
	cpe->http = http;
	evcpe_info(__func__, "accepting connection request: %s", cpe->creq_url->uri);
	evhttp_set_gencb(cpe->http, evcpe_creq_cb, cpe);

finally:
	return rc;
}

void evcpe_send_error(struct evcpe *cpe, enum evcpe_error_type type,
		int code, const char *reason)
{
	evcpe_error(__func__, "%d type error: %d %s", type, code, reason);
	if (cpe->error_cb)
		(*cpe->error_cb)(cpe, type, code, reason, cpe->cbarg);
}

int evcpe_retry_session(struct evcpe *cpe)
{
	int rc, base, secs;

	if (evtimer_pending(&cpe->retry_ev, NULL)) {
		evcpe_error(__func__, "another session retry has been scheduled");
		rc = EINVAL;
		goto finally;
	}

	if (cpe->retry_count == 0) {
		secs = 0;
	} else {
		switch (cpe->retry_count) {
		case 1:
			base = 5;
			break;
		case 2:
			base = 10;
			break;
		case 3:
			base = 20;
			break;
		case 4:
			base = 40;
			break;
		case 5:
			base = 80;
			break;
		case 6:
			base = 160;
			break;
		case 7:
			base = 320;
			break;
		case 8:
			base = 640;
			break;
		case 9:
			base = 1280;
			break;
		default:
			base = 2560;
			break;
		}
		secs = base + rand() % base;
	}

	evcpe_info(__func__, "scheduling session retry in %d second(s)", secs);

	evutil_timerclear(&cpe->retry_tv);
	cpe->retry_tv.tv_sec = secs;
	if ((rc = event_add(&cpe->retry_ev, &cpe->retry_tv)))
		evcpe_error(__func__, "failed to add timer event");

finally:
	return rc;
}

int evcpe_start(struct evcpe *cpe)
{
	int rc;

	if (!cpe->repo) {
		evcpe_error(__func__, "evcpe is not initialized");
		rc = EINVAL;
		goto finally;
	}

	evcpe_info(__func__, "starting evcpe");

	if ((rc = evcpe_repo_add_event(cpe->repo, "1 BOOT", ""))) {
		evcpe_error(__func__, "failed to add boot event");
		goto finally;
	}
	if (cpe->creq_url && (rc = evcpe_bind(cpe))) {
		evcpe_error(__func__, "failed to bind for connection request");
		goto finally;
	}
	evtimer_set(&cpe->retry_ev, evcpe_start_session_cb, cpe);
	if ((rc = evcpe_start_session(cpe))) {
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

void evcpe_start_session_cb(int fd, short ev, void *arg)
{
	int rc;
	struct evcpe *cpe = arg;
	if (!cpe->session && (rc = evcpe_start_session(cpe))) {
		evcpe_error(__func__, "failed to start session");
	}
	if (event_initialized(&cpe->periodic_ev) &&
			!evtimer_pending(&cpe->periodic_ev, NULL)) {
		evcpe_info(__func__, "scheduling periodic inform in %ld second(s)",
				cpe->periodic_tv.tv_sec);
		if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv)))
			evcpe_error(__func__, "failed to schedule periodic inform");
	}
}

int evcpe_start_session(struct evcpe *cpe)
{
	int rc;
	struct evcpe_inform *inform;
	struct evcpe_msg *msg;
	const char *hostname, *address;
	u_short port;
	struct evhttp_connection *conn;

	if (cpe->session) {
		evcpe_error(__func__, "session in progress");
		rc = EINVAL;
		goto finally;
	}

	evcpe_info(__func__, "starting session");

	msg = NULL;
	conn = NULL;

	hostname = cpe->proxy_url ? cpe->proxy_url->host : cpe->acs_url->host;
	port = cpe->proxy_url ? cpe->proxy_url->port : cpe->acs_url->port;
	if (!(address = evcpe_dns_cache_get(&cpe->dns_cache, hostname))) {
		evcpe_info(__func__, "hostname not resolved: %s", hostname);
		cpe->retry_count ++;
		if ((rc = evcpe_retry_session(cpe)))
			evcpe_error(__func__, "failed to schedule session retry");
		goto finally;
	}
	if (!(msg = evcpe_msg_new())) {
		evcpe_error(__func__, "failed to create evcpe_msg");
		rc = ENOMEM;
		goto exception;
	}
	if (!(msg->session = evcpe_ltoa(random()))) {
		evcpe_error(__func__, "failed to create session string");
		rc = ENOMEM;
		goto exception;
	}
	msg->major = 1;
	msg->minor = 0;
	if (!(msg->data = inform = evcpe_inform_new())) {
		evcpe_error(__func__, "failed to create inform request");
		rc = ENOMEM;
		goto exception;
	}
	msg->type = EVCPE_MSG_REQUEST;
	msg->method_type = EVCPE_INFORM;
	if ((rc = evcpe_repo_to_inform(cpe->repo, inform))) {
		evcpe_error(__func__, "failed to prepare inform message");
		goto exception;
	}
	inform->retry_count = cpe->retry_count;

	if (!(conn = evhttp_connection_new(address, port))) {
		evcpe_error(__func__, "failed to create evhttp_connection");
		rc = ENOMEM;
		goto exception;
	}
	evhttp_connection_set_base(conn, cpe->evbase);
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);
	if (!(cpe->session = evcpe_session_new(conn, cpe->acs_url,
			evcpe_session_message_cb, cpe))) {
		evcpe_error(__func__, "failed to create evcpe_session");
		rc = ENOMEM;
		goto exception;
	}
	evcpe_session_set_close_cb(cpe->session, evcpe_session_terminate_cb, cpe);
	if ((rc = evcpe_session_send(cpe->session, msg))) {
		evcpe_error(__func__, "failed to send inform message");
		goto exception;
	}
	if ((rc = evcpe_session_start(cpe->session))) {
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}
	rc = 0;
	goto finally;

exception:
	if (msg) evcpe_msg_free(msg);
	if (conn) evhttp_connection_free(conn);
	if (cpe->session) {
		evcpe_session_free(cpe->session);
		cpe->session = NULL;
	}

finally:
	return rc;
}

void evcpe_session_terminate_cb(struct evcpe_session *session, int rc,
		void *arg)
{
	struct evcpe *cpe = arg;
	evhttp_connection_free(session->conn);
	evcpe_session_free(cpe->session);
	cpe->session = NULL;
	if (rc) {
		evcpe_error(__func__, "session failed: %d - %s", rc, strerror(rc));
		cpe->retry_count ++;
		if (evcpe_retry_session(cpe)) {
			evcpe_error(__func__, "failed to schedule session retry");
		}
	} else {
		cpe->retry_count = 0;
	}
}

void evcpe_session_message_cb(struct evcpe_session *session,
		enum evcpe_msg_type type, enum evcpe_method_type method_type,
				void *request, void *response, void *arg)
{
	int rc;
	struct evcpe *cpe = arg;

	evcpe_info(__func__, "handling %s %s message from ACS",
			evcpe_method_type_to_str(method_type), evcpe_msg_type_to_str(type));

	switch(type) {
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_handle_request(cpe, session, method_type, request))) {
			evcpe_error(__func__, "failed to handle request");
			goto close_session;
		}
		break;
	case EVCPE_MSG_RESPONSE:
		if ((rc = evcpe_handle_response(cpe, method_type, request, response))) {
			evcpe_error(__func__, "failed to handle response");
			goto close_session;
		}
		break;
	case EVCPE_MSG_FAULT:
		evcpe_error(__func__, "CWMP fault encountered: %d - %s",
				((struct evcpe_fault *)request)->code,
				((struct evcpe_fault *)request)->string);
		// TODO: notifies
		goto close_session;
	default:
		evcpe_error(__func__, "unexpected message type: %d", type);
		rc = EINVAL;
		goto close_session;
	}
	return;

close_session:
	evcpe_session_close(cpe->session, rc);
}

int evcpe_handle_request(struct evcpe *cpe, struct evcpe_session *session,
		enum evcpe_method_type method_type, void *request)
{
	int rc;
	struct evcpe_msg *msg;
	struct evcpe_fault *fault;

	if (!(msg = evcpe_msg_new())) {
		evcpe_error(__func__, "failed to create evcpe_msg");
		rc = ENOMEM;
		goto finally;
	}
	msg->method_type = method_type;
	switch(method_type) {
	case EVCPE_GET_RPC_METHODS:
		rc = evcpe_handle_get_rpc_methods(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_NAMES:
		rc = evcpe_handle_get_param_names(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_ATTRIBUTES:
		rc = evcpe_handle_get_param_attrs(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_VALUES:
		rc = evcpe_handle_get_param_values(cpe, request, msg);
		break;
	case EVCPE_ADD_OBJECT:
		rc = evcpe_handle_add_object(cpe, request, msg);
		break;
	case EVCPE_SET_PARAMETER_VALUES:
		rc = evcpe_handle_set_param_values(cpe, request, msg);
		break;
	default:
		evcpe_error(__func__, "unexpected method type: %s",
				evcpe_method_type_to_str(method_type));
		rc = EVCPE_CPE_METHOD_NOT_SUPPORTED;
		break;
	}
	if (rc) {
		if (!(msg->data = fault = evcpe_fault_new())) {
			rc = ENOMEM;
			goto finally;
		}
		msg->type = EVCPE_MSG_FAULT;
		if (rc >= EVCPE_CPE_FAULT_MIN && rc <= EVCPE_CPE_FAULT_MAX)
			fault->code = rc;
//		else if (rc == ENOMEM)
//			fault->code = EVCPE_CPE_RESOUCES_EXCEEDS;
		else
			fault->code = EVCPE_CPE_INTERNAL_ERROR;
	}
	if ((rc = evcpe_session_send(session, msg))) {
		evcpe_error(__func__, "failed to send CWMP %s message",
				evcpe_method_type_to_str(msg->method_type));
		evcpe_msg_free(msg);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_response(struct evcpe *cpe,
		enum evcpe_method_type method_type, void *request, void *response)
{
	int rc;

	switch(method_type) {
	case EVCPE_INFORM:
		if ((rc = evcpe_handle_inform_response(cpe, request, response)))
			goto finally;
		break;
	default:
		evcpe_error(__func__, "unexpected method type: %d", method_type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_get_rpc_methods(struct evcpe *cpe,
		struct evcpe_get_rpc_methods *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_get_rpc_methods_response *method;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = method = evcpe_get_rpc_methods_response_new())) {
		evcpe_error(__func__, "failed to create "
				"evcpe_get_rpc_methods_response");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"GetRPCMethods"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"GetParameterNames"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"GetParameterValues"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"SetParameterValues"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"GetParameterAttributes"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"SetParameterAttributes"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"AddObject"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"DeleteObject"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"Download"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"Upload"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"Reboot"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"FactoryReset"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"GetQueuedTransfers"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
			"ScheduleInform"))) {
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_get_param_names(struct evcpe *cpe,
		struct evcpe_get_param_names *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_get_param_names_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_get_param_names_response_new())) {
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_names_response");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_repo_to_param_info_list(cpe->repo,
			req->parameter_path, &resp->parameter_list, req->next_level))) {
		evcpe_error(__func__, "failed to get param names: %s",
				req->parameter_path);
		evcpe_get_param_names_response_free(resp);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_get_param_values(struct evcpe *cpe,
		struct evcpe_get_param_values *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_param_name *param;
	struct evcpe_get_param_values_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_get_param_values_response_new())) {
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_values_response");
		rc = ENOMEM;
		goto finally;
	}
	TAILQ_FOREACH(param, &req->parameter_names.head, entry) {
		if ((rc = evcpe_repo_to_param_value_list(cpe->repo,
				param->name, &resp->parameter_list))) {
			evcpe_error(__func__, "failed to get param values: %s",
					param->name);
			evcpe_get_param_values_response_free(resp);
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_get_param_attrs(struct evcpe *cpe,
		struct evcpe_get_param_attrs *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_param_name *param;
	struct evcpe_get_param_attrs_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_get_param_attrs_response_new())) {
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_attrs_response");
		rc = ENOMEM;
		goto finally;
	}
	TAILQ_FOREACH(param, &req->parameter_names.head, entry) {
		if ((rc = evcpe_repo_to_param_attr_list(cpe->repo,
				param->name, &resp->parameter_list))) {
			evcpe_error(__func__, "failed to get param values: %s",
					param->name);
			evcpe_get_param_attrs_response_free(resp);
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_add_object(struct evcpe *cpe,
		struct evcpe_add_object *req,
		struct evcpe_msg *msg)
{
	int rc;
	unsigned int index;
	struct evcpe_add_object_response *resp;

	if ((rc = evcpe_repo_add_obj(cpe->repo, req->object_name, &index))) {
		evcpe_error(__func__, "failed to add object: %s", req->object_name);
		goto finally;
	}
	// TODO: set ParameterKey
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_add_object_response_new())) {
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->instance_number = index;
	resp->status = 0;
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_set_param_values(struct evcpe *cpe,
		struct evcpe_set_param_values *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_set_param_value *param;
	struct evcpe_set_param_values_response *resp;

	TAILQ_FOREACH(param, &req->parameter_list.head, entry) {
		if ((rc = evcpe_repo_set(cpe->repo, param->name,
				param->data, param->len))) {
			evcpe_error(__func__, "failed to set param: %s", param->name);
			// TODO: error codes
			goto finally;
		}
	}
	// TODO: set ParameterKey
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_set_param_values_response_new())) {
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_values_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->status = 0;
	rc = 0;

finally:
	return rc;
}

int evcpe_handle_inform_response(struct evcpe *cpe,
		struct evcpe_inform *req, struct evcpe_inform_response *resp)
{
	if (resp->max_envelopes != 1) {
		evcpe_error(__func__, "invalid max envelopes: %d", resp->max_envelopes);
		return EPROTO;
	}
	evcpe_repo_del_event(cpe->repo, "0 BOOTSTRAP");
	evcpe_repo_del_event(cpe->repo, "1 BOOT");
	return 0;
}

int evcpe_dns_entry_resolve(struct evcpe_dns_entry *entry, const char *hostname)
{
	int rc;
	evcpe_debug(__func__, "resolving DNS: %s", hostname);
	if ((rc = evdns_resolve_ipv4(hostname, 0, evcpe_dns_cb, entry)))
		evcpe_error(__func__, "failed to resolve IPv4 address: %s",
				entry->name);
	return rc;
}

int evcpe_dns_add(struct evcpe *cpe, const char *hostname)
{
	int rc;
	struct evcpe_dns_entry *entry;
	struct in_addr addr;

	evcpe_debug(__func__, "adding hostname: %s", hostname);

	if ((rc = evcpe_dns_cache_add(&cpe->dns_cache, hostname, &entry))) {
		evcpe_error(__func__, "failed to create DNS entry: %s", hostname);
		goto finally;
	}
	if (inet_aton(hostname, &addr)) {
		if (!(entry->address = strdup(hostname))) {
			evcpe_error(__func__, "failed to duplicate address");
			rc = ENOMEM;
			evcpe_dns_cache_remove(&cpe->dns_cache, entry);
			goto finally;
		}
	} else {
		if ((rc = evcpe_dns_entry_resolve(entry, entry->name))) {
			evcpe_error(__func__, "failed to resolve entry: %s", hostname);
			goto finally;
		}
		evtimer_set(&entry->ev, evcpe_dns_timer_cb, entry);
		event_base_set(cpe->evbase, &entry->ev);
	}
	rc = 0;

finally:
	return rc;
}

void evcpe_dns_timer_cb(int fd, short event, void *arg)
{
	if (evcpe_dns_entry_resolve(arg, ((struct evcpe_dns_entry *)arg)->name))
		evcpe_error(__func__, "failed to start DNS resolution: %s",
				((struct evcpe_dns_entry *)arg)->name);
}

void evcpe_dns_cb(int result, char type, int count, int ttl,
    void *addresses, void *arg)
{
	struct evcpe_dns_entry *entry = arg;
	const char *address;

	evcpe_debug(__func__, "starting DNS callback");

	switch(result) {
	case DNS_ERR_NONE:
		break;
	case DNS_ERR_SERVERFAILED:
	case DNS_ERR_FORMAT:
	case DNS_ERR_TRUNCATED:
	case DNS_ERR_NOTEXIST:
	case DNS_ERR_NOTIMPL:
	case DNS_ERR_REFUSED:
	case DNS_ERR_TIMEOUT:
	default:
		evcpe_error(__func__, "DNS resolution failed: %d", result);
		goto exception;
	}

	evcpe_debug(__func__, "type: %d, count: %d, ttl: %d: ", type, count, ttl);
	switch (type) {
	case DNS_IPv6_AAAA: {
		// TODO
		break;
	}
	case DNS_IPv4_A: {
		struct in_addr *in_addrs = addresses;
		/* a resolution that's not valid does not help */
		if (ttl < 0) {
			evcpe_error(__func__, "invalid DNS TTL: %d", ttl);
			goto exception;
		}
		if (count == 0) {
			evcpe_error(__func__, "zero DNS address count");
			goto exception;
		} else if (count == 1) {
			address = inet_ntoa(in_addrs[0]);
		} else {
			address = inet_ntoa(in_addrs[rand() % count]);
		}
		if (!(entry->address = strdup(address))) {
			evcpe_error(__func__, "failed to duplicate address string");
			goto exception;
		}
		evcpe_debug(__func__, "address resolved for entry \"%s\": %s",
				entry->name, address);
		entry->tv.tv_sec = ttl;
		break;
	}
	case DNS_PTR:
		/* may get at most one PTR */
		if (count != 1) {
			evcpe_error(__func__, "invalid PTR count: %d", count);
			goto exception;
		}
		address = *(char **)addresses;
		if (evcpe_dns_entry_resolve(entry, address)) {
			evcpe_error(__func__, "failed to start DNS resolve: %s", address);
			goto exception;
		}
		break;
	default:
		evcpe_error(__func__, "unexpected type: %d", type);
		goto exception;
	}
	goto finally;

exception:
	entry->tv.tv_sec = 0;

finally:
	evcpe_info(__func__, "next DNS resolution in %ld seconds: %s",
			entry->tv.tv_sec, entry->name);
	if (event_add(&entry->ev, &entry->tv)) {
		evcpe_error(__func__, "failed to schedule DNS resolution");
	}
}
