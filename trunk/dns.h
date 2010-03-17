// $Id$

#ifndef EVCPE_DNS_H_
#define EVCPE_DNS_H_

#include "dns_cache.h"

int evcpe_dns_add(struct evcpe_dns_cache *cache, const char *hostname);

const char *evcpe_dns_resolve(struct evcpe_dns_cache *cache, const char *hostname);

int evcpe_dns_clear(struct evcpe_dns_cache *cache);

#endif /* EVCPE_DNS_H_ */
