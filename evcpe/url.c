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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "util.h"
#include "log.h"

#include "url.h"

enum evcpe_url_state {
	EVCPE_URL_PROTO,
	EVCPE_URL_USER,
	EVCPE_URL_PASS,
	EVCPE_URL_HOST,
	EVCPE_URL_PORT,
	EVCPE_URL_PATH
};

u_short evcpe_url_default_port(const char *protocol)
{
	if (!strcmp("https", protocol)) {
		return 443;
	} else if (!strcmp("http", protocol)) {
		return 80;
	} else {
		return 0;
	}
}

struct evcpe_url *evcpe_url_new(void)
{
	struct evcpe_url *url;

	evcpe_debug(__func__, "constructing evcpe_url");

	if (!(url = calloc(1, sizeof(struct evcpe_url)))) {
		evcpe_error(__func__, "failed to calloc evcpe_url");
		return NULL;
	}
	return url;
}

void evcpe_url_free(struct evcpe_url *url)
{
	if (!url) return;
	evcpe_debug(__func__, "destructing evcpe_url");
	evcpe_url_reset(url);
	free(url);
}

void evcpe_url_reset(struct evcpe_url *url)
{
	if (url->protocol) free(url->protocol);
	if (url->username) free(url->username);
	if (url->password) free(url->password);
	if (url->host) free(url->host);
	if (url->uri) free(url->uri);
	bzero(url, sizeof(struct evcpe_url));
}

static int evcpe_url_set(char **ptr, const char *str, unsigned len)
{
	if (!(*ptr = malloc(len + 1)))
		return ENOMEM;
	memcpy(*ptr, str, len);
	*(*ptr + len) = '\0';
	return 0;
}

//int evcpe_url_from_str(struct evcpe_url *url, const char *str)
//{
//	int rc;
//	enum evcpe_url_state state;
//	const char *start, *ptr;
//
//	evcpe_url_reset(url);
//	for (state = EVCPE_URL_PROTO, start = ptr = str; *ptr != '\0'; ptr ++) {
//		switch(state) {
//		case EVCPE_URL_PROTO:
//			if (*ptr == ':') {
//				if (*(ptr + 1) != '/')
//					goto invalid;
//				if (*(ptr + 2) != '/')
//					goto invalid;
//			}
//		case EVCPE_URL_USER:
//		case EVCPE_URL_PASS:
//		case EVCPE_URL_HOST:
//		case EVCPE_URL_PORT:
//		case EVCPE_URL_PATH:
//		default:
//			rc = -1;
//			goto finally;
//		}
//	}
//	if (!url->protocol)
//		goto invalid;
//	if (!url->host && !url->uri)
//		goto invalid;
//	rc = 0;
//	goto finally;
//
//invalid:
//	rc = EINVAL;
//
//finally:
//	return rc;
//}

int evcpe_url_from_str(struct evcpe_url *url, const char *str)
{
	int rc;
	long val;
	const char *start, *end, *middle;

	evcpe_url_reset(url);
	start = str;
	if (!(end = strchr(start, ':')))
		goto invalid;
	if (*(end + 1) != '/')
		goto invalid;
	if (*(end + 2) != '/')
		goto invalid;
	if ((rc = evcpe_url_set(&url->protocol, start, end - start)))
		goto finally;
	if (*(start = end + 3) == '\0')
		goto invalid;
	if ((end = strchr(start, '@'))) {
		if ((middle = strchr(start, ':'))) {
			if ((rc = evcpe_url_set(&url->username, start, middle - start)))
				goto finally;
			if ((rc = evcpe_url_set(&url->password, middle + 1, end - middle - 1)))
				goto finally;
		} else {
			if ((rc = evcpe_url_set(&url->username, start, end - start)))
				goto finally;
		}
		start = end + 1;
	}
	if ((end = strchr(start, '/'))) {
		if ((rc = evcpe_url_set(&url->uri, end, strlen(end))))
			goto finally;
		if (start == end) {
			rc = 0;
			goto finally;
		}
	} else {
		end = strchr(start, '\0');
	}
	if ((middle = strchr(start, ':')) && middle < end) {
		if ((rc = evcpe_atol(middle + 1, end - middle - 1, &val)))
			goto finally;
		if (val < 0 || val > 65535)
			goto invalid;
		url->port = val;
		if ((rc = evcpe_url_set(&url->host, start, middle - start)))
			goto finally;
	} else {
		url->port = evcpe_url_default_port(url->protocol);
		if ((rc = evcpe_url_set(&url->host, start, end - start)))
			goto finally;
	}

	rc = 0;
	goto finally;

invalid:
	rc = EINVAL;

finally:
	return rc;
}
