// $Id$

#ifndef EVCPE_UTIL_H_
#define EVCPE_UTIL_H_

#include <event.h>

#include "evcpe-internal.h"

int evcpe_is_ipaddr(const char *address);

int evcpe_add_buffer(struct evbuffer *buffer, const char *fmt, ...) EVCPE_CHKFMT(2,3);

int evcpe_encode_base64(struct evbuffer *buffer, u_char *data, unsigned len);

int evcpe_strcmp(const char *a, unsigned alen, const char *b, unsigned blen);

inline int evcpe_strncmp(const char *a, const char *b, unsigned blen);

int evcpe_atol(const char *text, unsigned len, long *value);

char *evcpe_ltoa(long value);

int evcpe_strdup(const char *string, unsigned len, char **ptr);

#endif /* EVCPE_UTIL_H_ */
