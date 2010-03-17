// $Id$

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <evdns.h>

#include "log.h"

#include "dns.h"

static void evcpe_dns_cb(int result, char type, int count, int ttl,
	    void *addresses, void *arg);

static void evcpe_dns_timer_cb(int fd, short event, void *arg);

static const char *evcpe_dns_resolve(const char *hostname);
