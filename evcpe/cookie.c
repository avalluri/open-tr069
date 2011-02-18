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

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

#include "cookie.h"

int evcpe_cookie_cmp(struct evcpe_cookie *a, struct evcpe_cookie *b)
{
	return strcmp(a->name, b->name);
}

RB_GENERATE(evcpe_cookies, evcpe_cookie, entry, evcpe_cookie_cmp);

static void evcpe_cookie_free(struct evcpe_cookie *cookie)
{
	free(cookie->name);
	free(cookie->value);
	free(cookie);
}

void evcpe_cookies_clear(struct evcpe_cookies *cookies)
{
	struct evcpe_cookie *cookie;

	evcpe_debug(__func__, "clearing cookies");

	while((cookie = RB_ROOT(cookies))) {
		RB_REMOVE(evcpe_cookies, cookies, cookie);
		evcpe_cookie_free(cookie);
	}
}

struct evcpe_cookie *evcpe_cookies_find(struct evcpe_cookies *cookies,
		const char *name)
{
	struct evcpe_cookie find;

	if (!cookies || !name) return NULL;

	find.name = (char *)name;
	return RB_FIND(evcpe_cookies, cookies, &find);
}

int evcpe_cookies_set(struct evcpe_cookies *cookies,
		const char *name, const char *value)
{
	int rc;
	struct evcpe_cookie *old, *cookie;

	if (!cookies || !name || !value) return EINVAL;

	evcpe_debug(__func__, "setting cookie: %s => %s", name, value);

	if (!(cookie = calloc(1, sizeof(struct evcpe_cookie)))) {
		evcpe_error(__func__, "failed to calloc evcpe_cookie");
		rc = ENOMEM;
		goto finally;
	}
	if (!(cookie->name = strdup(name))) {
		evcpe_error(__func__, "failed to duplicate name: %s", name);
		evcpe_cookie_free(cookie);
		rc = ENOMEM;
		goto finally;
	}
	if (!(cookie->value = strdup(value))) {
		evcpe_error(__func__, "failed to duplicate value: %s", value);
		evcpe_cookie_free(cookie);
		rc = ENOMEM;
		goto finally;
	}
	if ((old = evcpe_cookies_find(cookies, name))) {
		RB_REMOVE(evcpe_cookies, cookies, old);
		evcpe_cookie_free(old);
	}
	RB_INSERT(evcpe_cookies, cookies, cookie);
	rc = 0;

finally:
	return rc;
}

int evcpe_cookies_set_from_header(struct evcpe_cookies *cookies,
		const char *header)
{
	int rc;
	char *dup, *ptr, *name, *value;

	if (!cookies || !header) return EINVAL;

	if (!(dup = strdup(header)))
		return ENOMEM;

	evcpe_debug(__func__, "setting cookie from header: %s", header);

	name = value = NULL;
	ptr = dup;
	while(*ptr) {
		if (!name) {
			if (*ptr != ' ')
				name = ptr;
		} else if (!value) {
			if (*ptr == '=') {
				value = ptr + 1;
				*ptr = '\0';
			}
		} else if (*ptr == ';' || *(ptr + 1) == '\0') {
			if (*ptr == ';')
				*ptr = '\0';
			if (strcmp("expires", name) && strcmp("path", name) &&
					strcmp("domain", name) && (rc = evcpe_cookies_set(
					cookies, name, value))) {
				evcpe_error(__func__, "failed to set cookie: %s=%s",
						name, value);
				goto finally;
			}
			name = value = NULL;
		}
		ptr ++;
	}
	rc = 0;

finally:
	free(dup);
	return rc;
}
