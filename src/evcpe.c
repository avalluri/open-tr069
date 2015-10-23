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

#include "evcpe-config.h"
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
#include "delete_object.h"
#include "set_param_values.h"
#include "set_param_attrs.h"
#include "download.h"

#include "evcpe.h"

static void evcpe_creq_cb(struct evhttp_request *req, void *arg);

static void evcpe_session_message_cb(evcpe_session *session,
		evcpe_msg_type_t type, evcpe_method_type_t method_type,
		void *request, void *response, void *arg);

static void evcpe_session_terminate_cb(evcpe_session *session,
		int code, void *arg);

static void evcpe_dns_cb(int result, char type, int count, int ttl,
		void *addresses, void *arg);

static inline void evcpe_dns_timer_cb(int fd, short event, void *arg);

static int evcpe_dns_entry_resolve(evcpe_dns_entry *entry,
		const char *hostname);

static int evcpe_dns_add(evcpe *cpe, const char *hostname);

static inline int evcpe_handle_request(evcpe *cpe,
		evcpe_session *session, evcpe_method_type_t method_type,
		void *request);

static inline int evcpe_handle_response(evcpe *cpe,
		evcpe_method_type_t method_type, void *request, void *response);

static inline int evcpe_handle_get_rpc_methods(evcpe *cpe,
		evcpe_get_rpc_methods *req, evcpe_msg *msg);

static inline int evcpe_handle_get_param_names(evcpe *cpe,
		evcpe_get_param_names *req, evcpe_msg *msg);

static inline int evcpe_handle_get_param_values(evcpe *cpe,
		evcpe_get_param_values *req, evcpe_msg *msg);

static inline int evcpe_handle_get_param_attrs(evcpe *cpe,
		evcpe_get_param_attrs *req, evcpe_msg *msg);

static inline int evcpe_handle_set_param_attrs(evcpe* cpe,
		evcpe_set_param_attrs* req, evcpe_msg* reply);

static inline int evcpe_handle_add_object(evcpe *cpe,
		evcpe_add_object *req, evcpe_msg *msg);

static inline int evcpe_handle_delete_object(evcpe *cpe,
		evcpe_delete_object* req, evcpe_msg* msg);

static inline int evcpe_handle_set_param_values(evcpe *cpe,
		evcpe_set_param_values *req, evcpe_msg *msg);

static inline int evcpe_handle_inform_response(evcpe *cpe,
		evcpe_inform *req, evcpe_inform_response *resp);

static inline int evcpe_handle_download(evcpe *cpe,
		evcpe_download *req, evcpe_msg *msg);

static int evcpe_retry_session(evcpe *cpe);

static void evcpe_start_session_cb(int fd, short event, void *arg);

static int evcpe_start_session(evcpe *cpe);

static void _on_download_complete(evcpe_download* req,
		evcpe_download_state_info* info, void* arg);


typedef struct _download_timer_cb_data {
	struct event* ev;
	evcpe_download* req;
	evcpe *cpe;
} download_timer_cb_data;

static
download_timer_cb_data* _download_timer_cb_data_new(struct event* ev,
		evcpe_download* req, evcpe *cpe) {
	download_timer_cb_data *info = calloc(1, sizeof(download_timer_cb_data));
	if (info) {
		info->ev = ev;
		info->req = req;
		info->cpe = cpe;
	}

	return info;
}

static
void _download_timer_cb_data_free(download_timer_cb_data* info)
{
	if (!info) return;

	if (info->ev) {
		if (evtimer_pending(info->ev, NULL)) evtimer_del(info->ev);
		event_free(info->ev);
	}
}

evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb_t cb, evcpe_error_cb_t error_cb, void *cbarg)
{
	evcpe *cpe;

	DEBUG("constructing evcpe");

	if ((cpe = calloc(1, sizeof(evcpe))) == NULL) {
		ERROR("failed to calloc evcpe");
		return NULL;
	}
	RB_INIT(&cpe->dns_cache);
	cpe->evbase = evbase;
	cpe->dnsbase = 	evdns_base_new(evbase, 1);
	cpe->cb = cb;
	cpe->error_cb = error_cb;
	cpe->cbarg = cbarg;
	cpe->pending_downloads = tqueue_new(NULL,
			(tqueue_free_func_t)_download_timer_cb_data_free);

	return cpe;
}

void evcpe_free(evcpe *cpe)
{
	if (cpe == NULL) return;

	evcpe_repo_set_download_cb(cpe->repo, NULL, NULL);

	TRACE("destructing evcpe");

	if (event_initialized(&cpe->retry_ev) &&
			evtimer_pending(&cpe->retry_ev, NULL)) {
		event_del(&cpe->retry_ev);
	}
	if (event_initialized(&cpe->periodic_ev) &&
			evtimer_pending(&cpe->periodic_ev, NULL)) {
		event_del(&cpe->periodic_ev);
	}

	if (cpe->pending_downloads) {
		tqueue_free(cpe->pending_downloads);
	}

	if (cpe->session) evcpe_session_free(cpe->session);
	if (cpe->http) evhttp_free(cpe->http);
	if (cpe->acs_url) evcpe_url_free(cpe->acs_url);
	if (cpe->proxy_url) evcpe_url_free(cpe->proxy_url);
	if (cpe->creq_url) evcpe_url_free(cpe->creq_url);
	if (cpe->dnsbase) evdns_base_free(cpe->dnsbase, 1);

	evcpe_dns_cache_clear(&cpe->dns_cache);
	free(cpe);
}

int evcpe_set(evcpe *cpe, evcpe_repo *repo)
{
	int rc, number;
	const char *value = NULL;
	unsigned int len;
	evcpe_obj *server_obj = NULL;
	unsigned i = 0;

	evcpe_repo_set_download_cb(repo, _on_download_complete, cpe);

	if ((rc = evcpe_repo_get_obj(repo, ".ManagementServer.", &server_obj))) {
		ERROR("Failed to locate ManagementServer object");
		return rc;
	}

	if (!(rc = evcpe_obj_get_attr_value(server_obj, "Authentication", &value,
			&len))) {
		if (!value || !len || !strcmp("NONE", value))
			cpe->acs_auth = EVCPE_AUTH_NONE;
		else if (!strcmp("BASIC", value))
			cpe->acs_auth = EVCPE_AUTH_NONE;
		else if (!strcmp("DIGEST", value))
			cpe->acs_auth = EVCPE_AUTH_NONE;
		else {
			ERROR("invalid authentication value: %s", value);
			rc = EINVAL;
			goto finally;
		}
	} else {
		ERROR("Failed to get 'Authentication' value: %d", rc);
		return rc;
	}

	if ((rc = evcpe_obj_get_attr_value(server_obj, "URL", &value, &len)) ||
			!value) {
		ERROR("failed to get ACS URL");
		goto finally;
	}
	if (!(cpe->acs_url = evcpe_url_new())) {
		ERROR("failed to create evcpe_url");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_url_from_str(cpe->acs_url, value))) {
		ERROR("failed to parse ACS URL: %s", value);
		goto finally;
	}
	if ((rc = evcpe_dns_add(cpe, cpe->acs_url->host))) {
		ERROR("failed to resolve ACS hostname");
		goto finally;
	}
	evcpe_obj_get_attr_value(server_obj, "Username", &cpe->acs_username, &len);
	evcpe_obj_get_attr_value(server_obj, "Password", &cpe->acs_password, &len);
	if (!(rc = evcpe_obj_get_attr_value(server_obj, "Timeout", &value, &len))) {
		if ((number = atoi(value)) < 0) {
			ERROR("invalid ACS timeout: %d, falling back to default timeout",
					number);
			cpe->acs_timeout = EVCPE_ACS_TIMEOUT;
		}
		else cpe->acs_timeout = number;
	} else {
		cpe->acs_timeout = EVCPE_ACS_TIMEOUT;
	}

	if (! evcpe_obj_get_attr_value(server_obj, "ProxyURL", &value, &len)
			&& value) {
		if (!(cpe->proxy_url = evcpe_url_new())) {
			ERROR("failed to create evcpe_url");
			rc = ENOMEM;
			goto finally;
		}
		if ((rc = evcpe_url_from_str(cpe->proxy_url, value))) {
			ERROR("failed to parse proxy URL: %s", value);
			goto finally;
		}
		if ((rc = evcpe_dns_add(cpe, cpe->proxy_url->host))) {
			ERROR("failed to resolve HTTP proxy hostname");
			goto finally;
		}
		evcpe_obj_get_attr_value(server_obj, "ProxyUsername",
				&cpe->proxy_username, &len);
		evcpe_obj_get_attr_value(server_obj, "ProxyPassword",
				&cpe->proxy_password, &len);
	}

	if (! evcpe_obj_get_attr_value(server_obj, "ConnectionRequestURL", &value,
			&len) && value) {
		if (!(cpe->creq_url = evcpe_url_new())) {
			ERROR("failed to create evcpe_url");
			rc = ENOMEM;
			goto finally;
		}
		if ((rc = evcpe_url_from_str(cpe->creq_url, value))) {
			ERROR("failed to parse ACS URL");
			goto finally;
		}
		evcpe_obj_get_attr_value(server_obj, "ConnectionRequestUsername",
				&cpe->creq_username, &len);
		evcpe_obj_get_attr_value(server_obj, "ConnectionRequestPassword",
				&cpe->creq_password, &len);
		if (! evcpe_obj_get_attr_value(server_obj, "ConnectionRequestInterval",
				&value, &len)) {
			if ((number = atoi(value)) < 0) {
				ERROR("invalid connection request interval: %d", number);
				rc = EINVAL;
				goto finally;
			}
			cpe->creq_interval = number;
		} else {
			cpe->creq_interval = EVCPE_CREQ_INTERVAL;
		}
	}

	if (! evcpe_obj_get_attr_value(server_obj, "PeriodicInformEnable",
			&value, &len) &&
			value && !strcmp("1", value)) {
		if (evcpe_obj_get_attr_value(server_obj,"PeriodicInformInterval",
				&value, &len) || !value) {
			ERROR("periodic inform interval was not set");
			rc = EINVAL;
			goto finally;
		}
		if ((number = atoi(value)) < 0) {
			ERROR("invalid periodic inform interval: %d", number);
			rc = EINVAL;
			goto finally;
		}
		evtimer_set(&cpe->periodic_ev, evcpe_start_session_cb, cpe);
		if ((rc = event_base_set(cpe->evbase, &cpe->periodic_ev))) {
			ERROR("failed to set event base");
			goto finally;
		}
		evutil_timerclear(&cpe->periodic_tv);
		cpe->periodic_tv.tv_sec = number;
		INFO("scheduling periodic inform in %ld second(s)",
				cpe->periodic_tv.tv_sec);
		if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv))) {
			ERROR("failed to schedule periodic inform");
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
	evcpe *cpe = arg;

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
			if (evcpe_repo_add_event(cpe->repo,
					EVCPE_EVENT_6_CONNECTION_REQUEST, "")) {
				ERROR("failed to add connection request event");
				evhttp_send_reply(req, 501, "Internal Server Error", NULL);
				return;
			}
			if (evcpe_start_session(cpe)) {
				ERROR("failed to start session");
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

int evcpe_bind(evcpe *cpe)
{
	int rc;
	struct evhttp *http;

	INFO("binding %s on port: %d",
			cpe->creq_url->protocol, cpe->creq_url->port);

	// TODO: SSL

	if (!(http = evhttp_new(cpe->evbase))) {
		ERROR("failed to create evhttp");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evhttp_bind_socket(http, "0.0.0.0", cpe->creq_url->port)) != 0) {
		ERROR("failed to bind evhttp on port:%d", cpe->creq_url->port);
		evhttp_free(http);
		goto finally;
	}

	if (cpe->http) evhttp_free(cpe->http);
	cpe->http = http;
	INFO("accepting connection request: %s", cpe->creq_url->uri);
	evhttp_set_gencb(cpe->http, evcpe_creq_cb, cpe);

	finally:
	return rc;
}

#if 0
static
void evcpe_send_error(evcpe *cpe, enum evcpe_error_type type,
		int code, const char *reason)
{
	ERROR("%d type error: %d %s", type, code, reason);
	if (cpe->error_cb)
		(*cpe->error_cb)(cpe, type, code, reason, cpe->cbarg);
}
#endif

int evcpe_retry_session(evcpe *cpe)
{
	int rc, base, secs;

	if (evtimer_pending(&cpe->retry_ev, NULL)) {
		ERROR("another session retry has been scheduled");
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

	INFO("scheduling session retry in %d second(s)", secs);

	evutil_timerclear(&cpe->retry_tv);
	cpe->retry_tv.tv_sec = secs;
	if ((rc = event_add(&cpe->retry_ev, &cpe->retry_tv)))
		ERROR("failed to add timer event");

finally:
	return rc;
}

int evcpe_start(evcpe *cpe, int bootstrap)
{
	int rc;

	if (!cpe->repo) {
		ERROR("evcpe is not initialized");
		rc = EINVAL;
		goto finally;
	}

	INFO("starting evcpe");

	if (bootstrap) {
		if ((rc = evcpe_repo_add_event(cpe->repo, EVCPE_EVENT_0_BOOTSTRAP, ""))) {
			ERROR("failed to add bootstrap event");
			goto finally;
		}
	} else {
		if ((rc = evcpe_repo_add_event(cpe->repo, EVCPE_EVENT_1_BOOT, ""))) {
			ERROR("failed to add boot event");
			goto finally;
		}
	}
	if (cpe->creq_url && (rc = evcpe_bind(cpe))) {
		ERROR("failed to bind for connection request");
		goto finally;
	}
	evtimer_set(&cpe->retry_ev, evcpe_start_session_cb, cpe);
	event_base_set(cpe->evbase, &cpe->retry_ev);
	if ((rc = evcpe_start_session(cpe))) {
		ERROR("failed to start session");
		goto finally;
	}
	rc = 0;

	finally:
	return rc;
}

void evcpe_start_session_cb(int fd, short ev, void *arg)
{
	int rc;
	evcpe *cpe = arg;
	if (!cpe->session && (rc = evcpe_start_session(cpe))) {
		ERROR("failed to start session");
	}
	if (event_initialized(&cpe->periodic_ev) &&
			!evtimer_pending(&cpe->periodic_ev, NULL)) {
		INFO("scheduling periodic inform in %ld second(s)",
				cpe->periodic_tv.tv_sec);
		if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv)))
			ERROR("failed to schedule periodic inform");
	}
}

int evcpe_start_session(evcpe *cpe)
{
	int rc;
	evcpe_inform *inform;
	evcpe_msg *msg;
	const char *hostname, *address;
	u_short port;
	struct evhttp_connection *conn;

	if (cpe->session) {
		ERROR("session in progress");
		rc = EINVAL;
		goto finally;
	}

	INFO("starting session");

	msg = NULL;
	conn = NULL;

	hostname = cpe->proxy_url ? cpe->proxy_url->host : cpe->acs_url->host;
	port = cpe->proxy_url ? cpe->proxy_url->port : cpe->acs_url->port;
	if (!(address = evcpe_dns_cache_get(&cpe->dns_cache, hostname))) {
		INFO("hostname not resolved: %s", hostname);
		cpe->retry_count ++;
		if ((rc = evcpe_retry_session(cpe)))
			ERROR("failed to schedule session retry :%d", rc);
		goto finally;
	}
	if (!(msg = evcpe_msg_new())) {
		ERROR("failed to create evcpe_msg");
		rc = ENOMEM;
		goto exception;
	}
	if (!(msg->session = evcpe_ltoa(random()))) {
		ERROR("failed to create session string");
		rc = ENOMEM;
		goto exception;
	}
	msg->major = 1;
	msg->minor = 0;
	if (!(msg->data = inform = evcpe_inform_new())) {
		ERROR("failed to create inform request");
		rc = ENOMEM;
		goto exception;
	}
	msg->type = EVCPE_MSG_REQUEST;
	msg->method_type = EVCPE_INFORM;
	if ((rc = evcpe_repo_to_inform(cpe->repo, inform))) {
		ERROR("failed to prepare inform message");
		goto exception;
	}
	inform->retry_count = cpe->retry_count;

	if (!(conn = evhttp_connection_new(address, port))) {
		ERROR("failed to create evhttp_connection");
		rc = ENOMEM;
		goto exception;
	}
	evhttp_connection_set_base(conn, cpe->evbase);
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);
	if (!(cpe->session = evcpe_session_new(conn, cpe->acs_url,
			evcpe_session_message_cb, cpe))) {
		ERROR("failed to create evcpe_session");
		rc = ENOMEM;
		goto exception;
	}
	evcpe_session_set_close_cb(cpe->session, evcpe_session_terminate_cb, cpe);
	if ((rc = evcpe_session_send(cpe->session, msg))) {
		ERROR("failed to send inform message");
		goto exception;
	}
	if ((rc = evcpe_session_start(cpe->session))) {
		ERROR("failed to start session");
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

void evcpe_session_terminate_cb(evcpe_session *session, int rc,
		void *arg)
{
	evcpe *cpe = arg;
	evhttp_connection_free(session->conn);
	evcpe_session_free(cpe->session);
	cpe->session = NULL;
	if (rc) {
		ERROR("session failed: %d - %s", rc, strerror(rc));
		cpe->retry_count ++;
		if (evcpe_retry_session(cpe)) {
			ERROR("failed to schedule session retry");
		}
	} else {
		cpe->retry_count = 0;
	}
}

void evcpe_session_message_cb(evcpe_session *session,
		evcpe_msg_type_t type, evcpe_method_type_t method_type,
		void *request, void *response, void *arg)
{
	int rc;
	evcpe *cpe = arg;

	INFO("handling %s %s message from ACS",
			evcpe_method_type_to_str(method_type), evcpe_msg_type_to_str(type));

	switch(type) {
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_handle_request(cpe, session, method_type, request))) {
			ERROR("failed to handle request");
			goto close_session;
		}
		break;
	case EVCPE_MSG_RESPONSE:
		if ((rc = evcpe_handle_response(cpe, method_type, request, response))) {
			ERROR("failed to handle response");
			goto close_session;
		}
		break;
	case EVCPE_MSG_FAULT:
		ERROR("CWMP fault encountered: %d - %s", ((evcpe_fault *)request)->code,
				((evcpe_fault *)request)->string);
		// TODO: notifies
		goto close_session;
	default:
		ERROR("unexpected message type: %d", type);
		rc = EINVAL;
		goto close_session;
	}
	return;

	close_session:
	evcpe_session_close(cpe->session, rc);
}

int evcpe_handle_request(evcpe *cpe, evcpe_session *session,
		evcpe_method_type_t method_type, void *request)
{
	int rc;
	evcpe_msg *reply = NULL;
	evcpe_fault *fault;


	if (!(reply = evcpe_msg_new())) {
		ERROR("failed to create evcpe_msg");
		rc = ENOMEM;
		goto finally;
	}
	reply->type = EVCPE_MSG_RESPONSE;
	reply->method_type = method_type;
	switch(method_type) {
	case EVCPE_GET_RPC_METHODS:
		rc = evcpe_handle_get_rpc_methods(cpe, request, reply);
		break;
	case EVCPE_GET_PARAMETER_NAMES:
		rc = evcpe_handle_get_param_names(cpe, request, reply);
		break;
	case EVCPE_GET_PARAMETER_ATTRIBUTES:
		rc = evcpe_handle_get_param_attrs(cpe, request, reply);
		break;
	case EVCPE_GET_PARAMETER_VALUES:
		rc = evcpe_handle_get_param_values(cpe, request, reply);
		break;
	case EVCPE_SET_PARAMETER_ATTRIBUTES:
		rc = evcpe_handle_set_param_attrs(cpe, request, reply);
		break;
	case EVCPE_SET_PARAMETER_VALUES:
		rc = evcpe_handle_set_param_values(cpe, request, reply);
		break;
	case EVCPE_ADD_OBJECT:
		rc = evcpe_handle_add_object(cpe, request, reply);
		break;
	case EVCPE_DELETE_OBJECT:
		rc = evcpe_handle_delete_object(cpe, request, reply);
		break;
	case EVCPE_DOWNLOAD:
		rc = evcpe_handle_download(cpe, request, reply);
		break;
	case EVCPE_REBOOT:
		//TODO: Handle Reboot

	default:
		ERROR("unexpected method type: %s",
				evcpe_method_type_to_str(method_type));
		rc = EVCPE_CPE_METHOD_NOT_SUPPORTED;
		break;
	}
	if (rc) {
		if (!(reply->data = fault = evcpe_fault_new())) {
			rc = ENOMEM;
			goto finally;
		}
		reply->type = EVCPE_MSG_FAULT;
		if (rc >= EVCPE_CPE_FAULT_MIN && rc <= EVCPE_CPE_FAULT_MAX)
			fault->code = rc;
		//		else if (rc == ENOMEM)
		//			fault->code = EVCPE_CPE_RESOUCES_EXCEEDS;
		else
			fault->code = EVCPE_CPE_INTERNAL_ERROR;
	}
	if ((rc = evcpe_session_send(session, reply))) {
		ERROR("failed to send CWMP %s message",
				evcpe_method_type_to_str(reply->method_type));
		evcpe_msg_free(reply);
		goto finally;
	}
	rc = 0;

	finally:
	return rc;
}

int evcpe_handle_response(evcpe *cpe,
		evcpe_method_type_t method_type, void *request, void *response)
{
	int rc;

	switch(method_type) {
	case EVCPE_INFORM:
		if ((rc = evcpe_handle_inform_response(cpe, request, response)))
			goto finally;
		break;
	default:
		ERROR("unexpected method type: %d", method_type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

	finally:
	return rc;
}
#if 0
static
struct MethodInfo {
	enum evcpe_method_type type;
	void* create();
	void  free(void* data);
	int   handle(evcpe *cpe, void* data, evcpe_msg* reply);
} supported_methods[] = {
	{ EVCPE_GET_RPC_METHODS, evcpe_get_rpc_methods_new,
	  evcpe_get_rpc_methods_free, evcpe_handle_get_rpc_methods
	},
	{ EVCPE_SET_PARAMETER_VALUES, evcpe_set_param_values_new,
	  evcpe_set_param_values_free, evcpe_handle_get_param_values
	},
	{ EVCPE_GET_PARAMETER_VALUES, evcpe_get_param_values_new,
	  evcpe_get_param_values_free, evcpe_handle_get_param_values
	},
	{ EVCPE_GET_PARAMETER_NAMES, evcpe_get_param_names_new,
	  evcpe_get_param_names_free, evcpe_handle_get_param_names
	},
	{ EVCPE_SET_PARAMETER_ATTRIBUTES, evcpe_set_param_attrs_new,
	  evcpe_set_param_attrs_free, evcpe_handle_set_param_attrs
	},
	{ EVCPE_GET_PARAMETER_ATTRIBUTES, evcpe_get_param_attrs_new,
	  evcpe_get_param_attrs_free, evcpe_handle_get_param_attrs
	},
	{ EVCPE_ADD_OBJECT, evcpe_add_object_new, evcpe_add_object_free,
	  evcpe_handle_add_object
	},
	{ EVCPE_DELETE_OBJECT, evcpe_delete_object_new, evcpe_delete_object_free,
	  evcpe_handle_delete_object
	},
	{ EVCPE_DOWNLOAD, evcpe_download_new, evcpe_download_free,
	  evcpe_handle_download
	},
	{ EVCPE_REBOOT, NULL, NULL, NULL }
};
#endif
int evcpe_handle_get_rpc_methods(evcpe *cpe, evcpe_get_rpc_methods *req,
		evcpe_msg *reply)
{
	int rc = 0, i;
	evcpe_get_rpc_methods_response *resp = NULL;
	static evcpe_method_type_t supported_methods[] = {
			EVCPE_GET_RPC_METHODS,
			EVCPE_GET_PARAMETER_NAMES,
			EVCPE_SET_PARAMETER_VALUES,
			EVCPE_GET_PARAMETER_VALUES,
			EVCPE_SET_PARAMETER_ATTRIBUTES,
			EVCPE_GET_PARAMETER_ATTRIBUTES,
			EVCPE_ADD_OBJECT,
			EVCPE_DELETE_OBJECT,
			EVCPE_DOWNLOAD,
			EVCPE_REBOOT
	};

	if (!(resp = evcpe_get_rpc_methods_response_new())) {
		ERROR("failed to create evcpe_get_rpc_methods_response");
		rc = ENOMEM;
		goto finally;
	}

	for (i = 0; i < sizeof(supported_methods)/sizeof(supported_methods[0]);
			i++) {
		if (! tqueue_insert(resp->method_list, (void*)supported_methods[i])) {
			evcpe_get_rpc_methods_response_free(resp);
			resp = NULL;
			rc = -1;
			ERROR("failed to add method");
			goto finally;
		}
	}
	reply->data = resp;

	finally:
	return rc;
}

int evcpe_handle_get_param_names(evcpe *cpe, evcpe_get_param_names *req,
		evcpe_msg *reply)
{
	int rc = 0;
	evcpe_get_param_names_response *resp = NULL;

	if (!(resp = evcpe_get_param_names_response_new())) {
		ERROR("failed to create evcpe_get_param_names_response");
		return ENOMEM;
	}
	if ((rc = evcpe_repo_to_param_info_list(cpe->repo, req->parameter_path,
			resp->parameter_list, req->next_level))) {
		ERROR("failed to get param names: %s", req->parameter_path);
		evcpe_get_param_names_response_free(resp);
		return rc;
	}
	reply->data = resp;

	return 0;
}

int evcpe_handle_get_param_values(evcpe *cpe, evcpe_get_param_values *req,
		evcpe_msg *reply)
{
	int rc = 0;
	tqueue_element* node = NULL;
	evcpe_get_param_values_response *resp = NULL;

	if (!(resp = evcpe_get_param_values_response_new())) {
		ERROR("failed to create evcpe_get_param_values_response");
		return ENOMEM;
	}

	TQUEUE_FOREACH(node, req->parameter_names) {
		if ((rc = evcpe_repo_to_param_value_list(cpe->repo,
				(char*)node->data, resp->parameter_list))) {
			ERROR("failed to get param values: %s", (char*)node->data);
			evcpe_get_param_values_response_free(resp);
			return rc;
		}
	}
	reply->data = resp;

	return 0;
}

int evcpe_handle_get_param_attrs(evcpe *cpe, evcpe_get_param_attrs *req,
		evcpe_msg *reply)
{
	int rc = 0;
	tqueue_element* node = NULL;
	evcpe_get_param_attrs_response *resp = NULL;

	if (!(resp = evcpe_get_param_attrs_response_new())) {
		ERROR("failed to create evcpe_get_param_attrs_response");
		return ENOMEM;
	}
	TQUEUE_FOREACH(node, req->parameter_names) {
		if ((rc = evcpe_repo_to_param_attr_list(cpe->repo,
				(char*)node->data, resp->parameter_list))) {
			ERROR("failed to get param values: %s", (char*)node->data);
			evcpe_get_param_attrs_response_free(resp);
			return rc;
		}
	}
	reply->data = resp;

	return 0;
}

int evcpe_handle_set_param_attrs(evcpe* cpe,
		evcpe_set_param_attrs* req, evcpe_msg* reply)
{
	int rc = 0;
	evcpe_set_param_attr* param = NULL;
	evcpe_set_param_attrs_response* resp = NULL;

	if (!(resp = evcpe_set_param_attrs_response_new())) {
		ERROR("failed to create evcpe_set_param_attrs_response");
		return ENOMEM;
	}

	if ((rc = evcpe_repo_set_param_atts(cpe->repo, req->parameter_list))) {
		free(resp);
		return rc;
	}
	resp->status = 1;
	reply->data = resp;

	return rc;
}

int evcpe_handle_add_object(evcpe *cpe,
		evcpe_add_object *req,
		evcpe_msg *reply)
{
	int rc = 0;
	unsigned int index;
	evcpe_add_object_response *resp;

	if ((rc = evcpe_repo_add_obj(cpe->repo, req->object_name, &index))) {
		ERROR("failed to add object: %s", req->object_name);
		return rc;
	}

	if (!(reply->data = resp = evcpe_add_object_response_new())) {
		ERROR("failed to create add_object_response");
		return ENOMEM;
	}

	evcpe_repo_set(cpe->repo, ".ManagementServer.ParameterKey",
			req->parameter_key, strlen(req->parameter_key));

	resp->instance_number = index;
	/* FIXME: Find a way to know if the object addition was validated
	 * but not applied yet!!!
	 */
	resp->status = 0;

	return 0;
}

static
int evcpe_handle_delete_object(evcpe* cpe, evcpe_delete_object* req,
		evcpe_msg* reply)
{
	int rc = 0;
	evcpe_delete_object_response* resp = NULL;

	if((rc = evcpe_repo_del_obj(cpe->repo, req->object_name))) {
		ERROR("Failed to delete object: %s", req->object_name);
		return rc;
	}

	if (!(resp = evcpe_delete_object_response_new())) return ENOMEM;

	evcpe_repo_set(cpe->repo, ".ManagementServer.ParameterKey",
			req->parameter_key, strlen(req->parameter_key));

	/* FIXME: Find a way to know if the object deletion was validated
	 * but not applied yet!!!
	 */
	resp->status = 0;

	reply->data = resp;

	return 0;
}

static
void _on_download_complete(evcpe_download* req, evcpe_download_state_info* info,
		void* arg)
{
	evcpe* cpe = arg;

	if (info->state == EVCPE_DOWNLOAD_APPLIED) {
	//TODO: Start new session if not already open, and send TransferComplete
	}
}

static
void _start_delayed_download(evutil_socket_t fd, short event, void *arg)
{
	download_timer_cb_data *info = arg;
	evcpe* cpe = info->cpe;
	evcpe_download* req = info->req;
	int rc = 0;

	tqueue_remove_data(cpe->pending_downloads, arg);

	if ((rc = evcpe_repo_download(cpe->repo, req)) > 1) {
		ERROR("Failed to download ");
	}
}

static
int evcpe_handle_download(evcpe* cpe, evcpe_download* req, evcpe_msg* reply)
{
	int rc = 0;
	evcpe_download_response* resp = NULL;

	if (tqueue_size(cpe->pending_downloads) == 3) {
		reply->data = NULL;
		return EVCPE_CPE_RESOUCES_EXCEEDS;
	}

	if (req->delay != 0) {
		struct timeval timeout = { req->delay, 0 };
		struct event* ev = evtimer_new(cpe->evbase, _start_delayed_download,
				(void*)cpe);
		download_timer_cb_data *info =
				_download_timer_cb_data_new(ev, req, cpe);
		tqueue_insert(cpe->pending_downloads, info);

		event_base_set(cpe->evbase, ev);
		evtimer_set(ev, _start_delayed_download, cpe);
		evtimer_add(ev, &timeout);
	} else {
		if((rc = evcpe_repo_download(cpe->repo, req)) > 1) {
			ERROR("Failed to download: %s", req->url->host);
			return rc;
		}
	}

	if (!(resp = evcpe_download_response_new())) return ENOMEM;

	resp->status = rc;
	reply->data = resp;

	return 0;
}

int evcpe_handle_set_param_values(evcpe *cpe,
		evcpe_set_param_values *req, evcpe_msg *reply)
{
	int rc = 0;
	tqueue_element* node = NULL;
	evcpe_set_param_values_response *resp = NULL;

	TQUEUE_FOREACH(node, req->parameter_list) {
		evcpe_param_value* param = (evcpe_param_value*)node->data;
		if ((rc = evcpe_repo_set(cpe->repo, param->name, param->data,
				param->len))) {
			ERROR("failed to set param: %s", param->name);
			// TODO: error codes
			goto finally;
		}
	}

	evcpe_repo_set(cpe->repo, ".ManagementServer.ParameterKey",
			req->parameter_key, strlen(req->parameter_key));

	if (!(reply->data = resp = evcpe_set_param_values_response_new())) {
		ERROR("failed to create evcpe_get_param_values_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->status = 0;
	rc = 0;

	finally:
	return rc;
}

int evcpe_handle_inform_response(evcpe *cpe, evcpe_inform *req,
		evcpe_inform_response *resp)
{
	if (resp->max_envelopes != 1) {
		ERROR("invalid max envelopes: %d", resp->max_envelopes);
		return EPROTO;
	}
#if 0
	evcpe_repo_del_event(cpe->repo, EVCPE_EVENT_0_BOOTSTRAP);
	evcpe_repo_del_event(cpe->repo, EVCPE_EVENT_1_BOOT);
#else
	// ??? Clear all pending events ???
	evcpe_repo_clear_pending_events(cpe->repo);
#endif
	return 0;
}

int evcpe_dns_entry_resolve(evcpe_dns_entry *entry, const char *hostname)
{
	DEBUG("resolving DNS: %s", hostname);
	if (evdns_base_resolve_ipv4(entry->dns_base, hostname, 0, evcpe_dns_cb,
			entry) == NULL) {
		ERROR("failed to resolve IPv4 address: %s", entry->name);
		return -1;
	}

	return 0;
}

int evcpe_dns_add(evcpe *cpe, const char *hostname)
{
	int rc;
	evcpe_dns_entry *entry;
	struct in_addr addr;

	DEBUG("adding hostname: %s", hostname);

	if ((rc = evcpe_dns_cache_add(&cpe->dns_cache, hostname, &entry))) {
		ERROR("failed to create DNS entry: %s", hostname);
		goto finally;
	}
	entry->dns_base = cpe->dnsbase;
	if (inet_aton(hostname, &addr)) {
		if (!(entry->address = strdup(hostname))) {
			ERROR("failed to duplicate address");
			rc = ENOMEM;
			evcpe_dns_cache_remove(&cpe->dns_cache, entry);
			goto finally;
		}
	} else {
		if ((rc = evcpe_dns_entry_resolve(entry, entry->name))) {
			ERROR("failed to resolve entry: %s", hostname);
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
	if (evcpe_dns_entry_resolve(arg, ((evcpe_dns_entry *)arg)->name))
		ERROR("failed to start DNS resolution: %s",
				((evcpe_dns_entry *)arg)->name);
}

void evcpe_dns_cb(int result, char type, int count, int ttl,
		void *addresses, void *arg)
{
	evcpe_dns_entry *entry = arg;
	const char *address;

	DEBUG("starting DNS callback");

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
		ERROR("DNS resolution failed: %d", result);
		goto exception;
	}

	DEBUG("type: %d, count: %d, ttl: %d: ", type, count, ttl);
	switch (type) {
	case DNS_IPv6_AAAA: {
		// TODO
		break;
	}
	case DNS_IPv4_A: {
		struct in_addr *in_addrs = addresses;
		/* a resolution that's not valid does not help */
		if (ttl < 0) {
			ERROR("invalid DNS TTL: %d", ttl);
			goto exception;
		}
		if (count == 0) {
			ERROR("zero DNS address count");
			goto exception;
		} else if (count == 1) {
			address = inet_ntoa(in_addrs[0]);
		} else {
			address = inet_ntoa(in_addrs[rand() % count]);
		}
		if (!(entry->address = strdup(address))) {
			ERROR("failed to duplicate address string");
			goto exception;
		}
		DEBUG("address resolved for entry \"%s\": %s", entry->name, address);
		entry->tv.tv_sec = ttl;
		break;
	}
	case DNS_PTR:
		/* may get at most one PTR */
		if (count != 1) {
			ERROR("invalid PTR count: %d", count);
			goto exception;
		}
		address = *(char **)addresses;
		if (evcpe_dns_entry_resolve(entry, address)) {
			ERROR("failed to start DNS resolve: %s", address);
			goto exception;
		}
		break;
	default:
		ERROR("unexpected type: %d", type);
		goto exception;
	}
	goto finally;

	exception:
	entry->tv.tv_sec = 0;

	finally:
	INFO("next DNS resolution in %ld seconds: %s",
			entry->tv.tv_sec, entry->name);
	if (event_add(&entry->ev, &entry->tv)) {
		ERROR("failed to schedule DNS resolution");
	}
}
