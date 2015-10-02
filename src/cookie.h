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

#ifndef EVCPE_COOKIE_H_
#define EVCPE_COOKIE_H_

#include <sys/tree.h>

typedef struct _evcpe_cookie {
	char *name;
	char *value;
	RB_ENTRY(_evcpe_cookie) entry;
} evcpe_cookie;

RB_HEAD(_evcpe_cookies, _evcpe_cookie);
typedef struct _evcpe_cookies evcpe_cookies;

int evcpe_cookie_cmp(evcpe_cookie *a, evcpe_cookie *b);

RB_PROTOTYPE(_evcpe_cookies, _evcpe_cookie, entry, evcpe_cookie_cmp);

void evcpe_cookies_clear(evcpe_cookies *cookies);

evcpe_cookie *evcpe_cookies_find(evcpe_cookies *cookies, const char *name);

int evcpe_cookies_set(evcpe_cookies *cookies, const char *name,
		const char *value);

int evcpe_cookies_set_from_header(evcpe_cookies *cookies,
		const char *header);

#endif /* EVCPE_COOKIE_H_ */
