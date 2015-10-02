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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <evhttp.h>

#include "evcpe-config.h"
#include "log.h"
#include "util.h"
#include "msg.h"
#include "method.h"

#include "session.h"

static
void evcpe_session_http_cb(struct evhttp_request *http_req, void *arg);

static
void evcpe_session_http_close_cb(struct evhttp_connection *conn, void *arg)
{
	ERROR("HTTP connection closed");
	evcpe_session_close(arg, ECONNRESET);
}

static
int evcpe_session_handle_incoming(evcpe_session *session,
		struct evbuffer *buffer)
{
	int rc;
	evcpe_msg *msg, *req;
	tqueue_element* node = NULL;

	if (!(msg = evcpe_msg_new())) {
		ERROR("failed to create evcpe_msg");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_msg_from_xml(msg, buffer))) {
		ERROR("failed to unmarshal response");
		goto finally;
	}
	session->hold_requests = msg->hold_requests;

	switch (msg->type) {
	case EVCPE_MSG_REQUEST:
		tqueue_insert(session->req_in, msg);
		(*session->cb)(session, msg->type, msg->method_type,
				msg->data, NULL, session->cbarg);
		break;
	case EVCPE_MSG_RESPONSE:
	case EVCPE_MSG_FAULT:
		if (!(node = tqueue_first(session->req_out))) {
			ERROR("no pending CPE request");
			rc = EPROTO;
			goto finally;
		}
		req = (evcpe_msg*)node->data;
		if (msg->type == EVCPE_MSG_RESPONSE
				&& msg->method_type != req->method_type) {
			ERROR("method of request/response doesn't match: "
					"%d != %d", req->method_type, msg->method_type);
			rc = EPROTO;
			goto finally;
		}
		(*session->cb)(session, msg->type, msg->method_type,
				req, msg->data, session->cbarg);
		tqueue_remove(session->req_out, node);
		evcpe_msg_free(msg);
		break;
	default:
		ERROR("unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

static
int evcpe_session_add_header(struct evkeyvalq *keyvalq,
		const char *key, const char *value)
{
	DEBUG("adding HTTP header: %s => %s", key, value);
	return evhttp_add_header(keyvalq, key, value);
}

static
int evcpe_session_send_do(evcpe_session *session, evcpe_msg *msg)
{
	int rc, len;
	char buffer[513];
	struct evhttp_request *req;
	evcpe_cookie *cookie;

	if (msg)
		INFO("sending CWMP %s message in HTTP request",
				evcpe_msg_type_to_str(msg->type));
	else
		INFO("sending empty HTTP request");

	if (!(req = evhttp_request_new(
			evcpe_session_http_cb, session))) {
		ERROR("failed to create evhttp_connection");
		rc = ENOMEM;
		goto finally;
	}
	if (msg && msg->data && (rc = evcpe_msg_to_xml(msg, req->output_buffer))) {
		ERROR("failed to create SOAP message");
		evhttp_request_free(req);
		goto finally;
	}
	req->major = 1;
	req->minor = 1;
	snprintf(buffer, sizeof(buffer), "%s:%d",
			session->acs->host, session->acs->port);
	if ((rc = evcpe_session_add_header(req->output_headers, "Host", buffer))) {
		ERROR("failed to add header: Host=\"%s\"", buffer);
		evhttp_request_free(req);
		goto finally;
	}
	if (!RB_EMPTY(&session->cookies)) {
		len = 0;
		RB_FOREACH(cookie, _evcpe_cookies, &session->cookies) {
			len += snprintf(buffer + len, sizeof(buffer) - len, "%s=%s; ",
					cookie->name, cookie->value);
		}
		if (len - 2 < sizeof(buffer))
			buffer[len - 2] = '\0';
		if ((rc = evcpe_session_add_header(req->output_headers,
				"Cookie", buffer))) {
			ERROR("failed to add header: Cookie=\"%s\"", buffer);
			evhttp_request_free(req);
			goto finally;
		}
	}
	if (msg && msg->data && (rc = evcpe_session_add_header(req->output_headers,
			"SOAPAction", ""))) {
		ERROR("failed to add header: SOAPAction=\"\"");
		evhttp_request_free(req);
		goto finally;
	}
	if ((rc = evcpe_session_add_header(req->output_headers,
			"User-Agent", "evcpe-"EVCPE_VERSION))) {
		ERROR("failed to add header: User-Agent=\"evcpe-"EVCPE_VERSION"\"");
		evhttp_request_free(req);
		goto finally;
	}
	if ((rc = evcpe_session_add_header(req->output_headers,
			"Content-Type", "text/xml"))) {
		ERROR("failed to add header: Content-Type=text/xml");
		evhttp_request_free(req);
		goto finally;
	}
	DEBUG("HTTP request content: %.*s",
			(int)evbuffer_get_length(req->output_buffer),
			evbuffer_pullup(req->output_buffer, -1));

	INFO("making HTTP request");

	if ((rc = evhttp_make_request(session->conn, req,
			EVHTTP_REQ_POST, session->acs->uri))) {
		ERROR("failed to make request");
		evhttp_request_free(req);
		goto finally;
	}
	evhttp_connection_set_closecb(session->conn, evcpe_session_http_close_cb,
			session);
	rc = 0;

finally:
	return rc;
}

static
void evcpe_session_http_cb(struct evhttp_request *http_req, void *arg)
{
	int rc;
	const char *cookies;
	evcpe_msg *msg;
	evcpe_session *session = arg;
	tqueue_element* node = NULL;

	if (0 == http_req->response_code) {
		INFO("session timed out");
		rc = ETIMEDOUT;
		goto close;
	}

	INFO("HTTP response code: %d", http_req->response_code);
	DEBUG("HTTP response content: %.*s",
			(int)evbuffer_get_length(http_req->input_buffer),
			evbuffer_pullup(http_req->input_buffer, -1));

	if ((cookies = evhttp_find_header(http_req->input_headers, "Set-Cookie"))
			&& (rc = evcpe_cookies_set_from_header(
					&session->cookies, cookies))) {
		ERROR("failed to set cookies: %s", cookies);
		goto close;
	}
	if (evbuffer_get_length(http_req->input_buffer) &&
			(rc = evcpe_session_handle_incoming(
					session, http_req->input_buffer))) {
		ERROR("failed to handle incoming data");
		goto close;
	}
	if ((node = tqueue_first(session->res_pending))) {
		msg = (evcpe_msg*)node->data;
		if ((rc = evcpe_session_send_do(session, msg))) {
			ERROR("failed to response ACS request");
			goto close;
		}
		tqueue_remove(session->res_pending, node);
	} else if (session->hold_requests) {
		if ((rc = evcpe_session_send_do(session, NULL))) {
			ERROR("failed to send empty HTTP request");
			goto close;
		}
	} else if ((node = tqueue_first(session->req_pending))) {
		msg = (evcpe_msg*)node->data;
		if ((rc = evcpe_session_send_do(session, msg))) {
			ERROR("failed to send CPE request");
			goto close;
		}
		tqueue_remove(session->req_pending, node);
		tqueue_insert(session->req_out, msg);
	} else if (!evbuffer_get_length(http_req->input_buffer)) {
		INFO("session termination criteria are met");
		goto close;
	} else {
		if ((rc = evcpe_session_send_do(session, NULL))) {
			ERROR("failed to send empty HTTP request");
			goto close;
		}
	}
	return;

close:
	evcpe_session_close(session, rc);
}

evcpe_session *evcpe_session_new(struct evhttp_connection *conn,
		evcpe_url *acs, evcpe_session_cb_t cb, void *cbarg)
{
	evcpe_session *session;

	DEBUG("constructing evcpe_session");

	if (!(session = calloc(1, sizeof(evcpe_session)))) {
		ERROR("failed to calloc evcpe_session");
		return NULL;
	}
	RB_INIT(&session->cookies);
	session->req_in = tqueue_new(NULL, (tqueue_free_func_t)evcpe_msg_free);
	session->req_out = tqueue_new(NULL, (tqueue_free_func_t)evcpe_msg_free);
	session->req_pending = tqueue_new(NULL, (tqueue_free_func_t)evcpe_msg_free);
	session->res_pending = tqueue_new(NULL, (tqueue_free_func_t)evcpe_msg_free);
	session->conn = conn;
	session->acs = acs;
	session->cb = cb;
	session->cbarg = cbarg;

	return session;
}

void evcpe_session_close(evcpe_session *session, int code)
{
	INFO("closing CWMP session");
	evhttp_connection_set_closecb(session->conn, NULL, NULL);
	if (session->close_cb)
		(*session->close_cb)(session, code, session->close_cbarg);
}

void evcpe_session_free(evcpe_session *session)
{
	if (!session) return;

	DEBUG("destructing evcpe_session");

	evcpe_cookies_clear(&session->cookies);
	tqueue_free(session->req_in);
	tqueue_free(session->req_out);
	tqueue_free(session->req_pending);
	tqueue_free(session->res_pending);
	if (session->conn) {
		evhttp_connection_set_closecb(session->conn, NULL, NULL);
	}
	free(session);
}

void evcpe_session_set_close_cb(evcpe_session *session,
		evcpe_session_close_cb_t close_cb, void *cbarg)
{
	session->close_cb = close_cb;
	session->close_cbarg = cbarg;
}

int evcpe_session_start(evcpe_session *session)
{
	int rc = 0;
	evcpe_msg *req;
	tqueue_element* node = NULL;

	if (!(node = tqueue_first(session->req_pending))) {
		ERROR("no pending CPE request");
		rc = EINVAL;
		goto finally;
	}
	req = (evcpe_msg*)node->data;
	if (req->method_type != EVCPE_INFORM) {
		ERROR("first CPE request must be an inform");
		rc = EINVAL;
		goto finally;
	}
	//req = TAILQ_FIRST(&session->req_pending);
	if ((rc = evcpe_session_send_do(session, req))) {
		ERROR("failed to send first request");
		goto finally;
	}
	node->data = NULL;
	tqueue_remove(session->req_pending, node);
	tqueue_insert(session->req_out, req);

finally:
	return rc;
}

int evcpe_session_send(evcpe_session *session, evcpe_msg *msg)
{
	int rc = 0;
	evcpe_msg *req = NULL;
	tqueue_element* node = NULL;

	if (!session || !msg) return EINVAL;

	switch (msg->type) {
	case EVCPE_MSG_REQUEST:
		tqueue_insert(session->req_pending, msg);
		break;
	case EVCPE_MSG_RESPONSE:
	case EVCPE_MSG_FAULT:
		if (!(node = tqueue_first(session->req_in))) {
			ERROR("no pending ACS request");
			rc = -1;
			goto finally;
		}
		req = (evcpe_msg*)node->data;
		if (req->method_type != msg->method_type) {
			ERROR("method type mismatch: %s != %s",
					evcpe_method_type_to_str(req->method_type),
					evcpe_method_type_to_str(msg->method_type));
			rc = -1;
			goto finally;
		}
		if (!(msg->session = strdup(req->session))) {
			ERROR("failed to duplicate session ID");
			rc = ENOMEM;
			goto finally;
		}
		tqueue_insert(session->res_pending, msg);
		tqueue_remove(session->req_in, node);
		break;
	default:
		ERROR("unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

