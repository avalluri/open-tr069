// $Id$

#ifndef EVCPE_PERSISTER_H_
#define EVCPE_PERSISTER_H_

#include <stdio.h>
#include <event.h>

struct evcpe_persister {
	struct event_base *evbase;
	struct evcpe_repo *repo;
	struct evbuffer *buffer;
//	struct event write_ev;
	struct event timer_ev;
	struct timeval timer_tv;
	const char *filename;
//	int fd;
//	size_t written;
};

struct evcpe_persister *evcpe_persister_new(struct event_base *evbase);

void evcpe_persister_free(struct evcpe_persister *persist);

int evcpe_persister_set(struct evcpe_persister *persist,
		struct evcpe_repo *repo, const char *filename);

#endif /* EVCPE_PERSISTER_H_ */
