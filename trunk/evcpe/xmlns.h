// $Id$

#ifndef EVCPE_XMLNS_H_
#define EVCPE_XMLNS_H_

#include <sys/types.h>
#include <sys/tree.h>

struct evcpe_xmlns {
	const char *name;
	unsigned name_len;
	const char *value;
	unsigned value_len;
	RB_ENTRY(evcpe_xmlns) entry;
};

RB_HEAD(evcpe_xmlns_table, evcpe_xmlns);

int evcpe_xmlns_cmp(struct evcpe_xmlns *a, struct evcpe_xmlns *b);

RB_PROTOTYPE(evcpe_xmlns_table, evcpe_xmlns, entry, evcpe_xmlns_cmp);

void evcpe_xmlns_table_clear(struct evcpe_xmlns_table *table);

int evcpe_xmlns_table_add(struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len);

struct evcpe_xmlns *evcpe_xmlns_table_find(struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len);

int evcpe_xmlns_table_get(struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len,
		const char **value, unsigned *value_len);

#endif /* EVCPE_XMLNS_H_ */
