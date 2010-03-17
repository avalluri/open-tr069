// $Id$

#ifndef EVCPE_DNS_CACHE_H_
#define EVCPE_DNS_CACHE_H_

#include <sys/tree.h>
#include <time.h>
#include <event.h>

struct evcpe_dns_entry {
	char *name;
	char *address;
	struct event ev;
	struct timeval tv;
	RB_ENTRY(evcpe_dns_entry) entry;
};

RB_HEAD(evcpe_dns_cache, evcpe_dns_entry);

int evcpe_dns_entry_cmp(struct evcpe_dns_entry *a, struct evcpe_dns_entry *b);

RB_PROTOTYPE(evcpe_dns_cache, evcpe_dns_entry, entry, evcpe_dns_entry_cmp);

int evcpe_dns_cache_add(struct evcpe_dns_cache *cache,
		const char *name, struct evcpe_dns_entry **entry);

struct evcpe_dns_entry *evcpe_dns_cache_find(
		struct evcpe_dns_cache *cache, const char *name);

const char *evcpe_dns_cache_get(struct evcpe_dns_cache *cache,
		const char *name);

inline void evcpe_dns_cache_remove(struct evcpe_dns_cache *cache,
		struct evcpe_dns_entry *entry);

void evcpe_dns_cache_clear(struct evcpe_dns_cache *cache);

#endif /* EVCPE_DNS_CACHE_H_ */
