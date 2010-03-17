// $Id: url.h 122 2009-01-11 08:48:04Z cedric $

#ifndef EVCPE_URL_H_
#define EVCPE_URL_H_

#include <sys/types.h>
#include <event.h>
#include <stdio.h>

struct evcpe_url {
	char *protocol;
	char *username;
	char *password;
	char *host;
	u_short port;
	char *uri;
};

u_short evcpe_url_default_port(const char *protocol);

struct evcpe_url *evcpe_url_new(void);

void evcpe_url_free(struct evcpe_url *url);

void evcpe_url_reset(struct evcpe_url *url);

int evcpe_url_from_str(struct evcpe_url *url, const char *str);

void evcpe_print_buffer(FILE *file, const char *msg, struct evbuffer *buffer);

#endif /* EVCPE_URL_H_ */
