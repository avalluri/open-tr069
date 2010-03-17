// $Id$

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"

#include "util.h"

int evcpe_is_ipaddr(const char *address)
{
	int i;

	for (i = 0; i < strlen(address); i++) {
		if (address[i] != '.' && (address[i] < '0' || address[i] > '9'))
			return 0;
	}
	return 1;
}

int evcpe_add_buffer(struct evbuffer *buffer, const char *fmt, ...)
{
	int rc;
	va_list ap;

	va_start(ap, fmt);
	if (evbuffer_add_vprintf(buffer, fmt, ap) < 0)
		rc = ENOMEM;
	else
		rc = 0;
	va_end(ap);

	return rc;
}

int evcpe_encode_base64(struct evbuffer *buffer, u_char *data, unsigned len)
{
	// TODO
	return -1;
}
//
//int evcpe_streql(const char *a, const char *b)
//{
//	return !evcpe_strcmp(a, strlen(a), b, strlen(b));
//}

int evcpe_strcmp(const char *a, unsigned alen, const char *b, unsigned blen)
{
	int result;
	unsigned len;

//	evcpe_trace(__func__, "comparing \"%.*s\" with \"%.*s\"", alen, a, blen, b);

	len = alen > blen ? blen : alen;
	result = strncmp(a, b, len);

	if (alen != blen && result == 0) {
		if (alen > blen)
			result = 1;
		else
			result = -1;
	}

	return result;
}

int evcpe_strncmp(const char *a, const char *b, unsigned blen)
{
	return evcpe_strcmp(a, strlen(a), b, blen);
}

int evcpe_atol(const char *text, unsigned len, long *value)
{
	int i, negative;

	if (len == 0) return EINVAL;
	if (text[0] == '-') {
		if (len == 1)
			return EINVAL;
		i = 1;
		negative = 1;
	} else {
		i = 0;
		negative = 0;
	}
	*value = 0;
	while (i < len) {
		if (text[i] < '0' || text[i] > '9') return EINVAL;
		if (negative)
			*value -= (text[i] - '0');
		else
			*value += (text[i] - '0');
		i ++;
		if (i != len)
			*value *= 10;
	}
	return 0;
}

char *evcpe_ltoa(long value)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%ld", value);
	return strdup(buffer);
}

int evcpe_strdup(const char *string, unsigned len, char **ptr)
{
	if (!(*ptr = malloc(len + 1))) {
//		evcpe_error(__func__, "failed to malloc: %d", len + 1);
		return ENOMEM;
	}
	memcpy(*ptr, string, len);
	(*ptr)[len] = '\0';
	return 0;
}
