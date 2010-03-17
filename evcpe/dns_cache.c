// $Id$

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"

#include "dns_cache.h"

static inline void evcpe_dns_entry_free(struct evcpe_dns_entry *entry);

int evcpe_dns_entry_cmp(struct evcpe_dns_entry *a, struct evcpe_dns_entry *b)
{
	return strcmp(a->name, b->name);
}

RB_GENERATE(evcpe_dns_cache, evcpe_dns_entry, entry, evcpe_dns_entry_cmp);

void evcpe_dns_entry_free(struct evcpe_dns_entry *entry)
{
	if (entry->name) free(entry->name);
	if (entry->address) free(entry->address);
	free(entry);
}

struct evcpe_dns_entry *evcpe_dns_cache_find(struct evcpe_dns_cache *cache,
		const char *name)
{
	struct evcpe_dns_entry find;
	find.name = (char *)name;
	return RB_FIND(evcpe_dns_cache, cache, &find);
}

void evcpe_dns_cache_remove(struct evcpe_dns_cache *cache,
		struct evcpe_dns_entry *entry)
{
	evcpe_debug(__func__, "removing DNS entry: %s", entry->name);
	RB_REMOVE(evcpe_dns_cache, cache, entry);
	evcpe_dns_entry_free(entry);
}

int evcpe_dns_cache_add(struct evcpe_dns_cache *cache,
		const char *hostname, struct evcpe_dns_entry **entry)
{
	int rc;

	evcpe_debug(__func__, "adding DNS entry: %s", hostname);

	if (!(*entry = calloc(1, sizeof(struct evcpe_dns_entry)))) {
		evcpe_error(__func__, "failed to calloc evcpe_dns_entry");
		rc = ENOMEM;
		goto finally;
	}
	if (!((*entry)->name = strdup(hostname))) {
		evcpe_error(__func__, "failed to duplicate hostname");
		rc = ENOMEM;
		evcpe_dns_entry_free(*entry);
		goto finally;
	}
	RB_INSERT(evcpe_dns_cache, cache, *entry);
	rc = 0;

finally:
	return rc;
}

const char *evcpe_dns_cache_get(struct evcpe_dns_cache *cache,
		const char *name)
{
	struct evcpe_dns_entry *entry;

	if (!cache || !name) return NULL;

	evcpe_debug(__func__, "getting DNS entry: %s", name);

	if ((entry = evcpe_dns_cache_find(cache, name))) {
		return entry->address;
	} else {
		return NULL;
	}
}

void evcpe_dns_cache_clear(struct evcpe_dns_cache *cache)
{
	struct evcpe_dns_entry *entry;
	evcpe_debug(__func__, "clearing all DNS entries");
	RB_FOREACH(entry, evcpe_dns_cache, cache) {
		evcpe_dns_cache_remove(cache, entry);
	}
}
