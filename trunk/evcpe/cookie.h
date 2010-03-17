// $Id$

#ifndef EVCPE_COOKIE_H_
#define EVCPE_COOKIE_H_

#include <sys/tree.h>

struct evcpe_cookie {
	char *name;
	char *value;
	RB_ENTRY(evcpe_cookie) entry;
};

RB_HEAD(evcpe_cookies, evcpe_cookie);

int evcpe_cookie_cmp(struct evcpe_cookie *a, struct evcpe_cookie *b);

RB_PROTOTYPE(evcpe_cookies, evcpe_cookie, entry, evcpe_cookie_cmp);

void evcpe_cookies_clear(struct evcpe_cookies *cookies);

struct evcpe_cookie *evcpe_cookies_find(struct evcpe_cookies *cookies,
		const char *name);

int evcpe_cookies_set(struct evcpe_cookies *cookies,
		const char *name, const char *value);

int evcpe_cookies_set_from_header(struct evcpe_cookies *cookies,
		const char *header);

#endif /* EVCPE_COOKIE_H_ */
