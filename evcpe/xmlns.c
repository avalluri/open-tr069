// $Id$

#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "log.h"

#include "xmlns.h"

int evcpe_xmlns_cmp(struct evcpe_xmlns *a, struct evcpe_xmlns *b)
{
	return evcpe_strcmp(a->name, a->name_len, b->name, b->name_len);
}

RB_GENERATE(evcpe_xmlns_table, evcpe_xmlns, entry, evcpe_xmlns_cmp);

void evcpe_xmlns_table_clear(struct evcpe_xmlns_table *table)
{
	struct evcpe_xmlns *ns;

	evcpe_trace(__func__, "clearing namespace table");

	while((ns = RB_MIN(evcpe_xmlns_table, table))) {
		RB_REMOVE(evcpe_xmlns_table, table, ns);
		free(ns);
	}
}

struct evcpe_xmlns *evcpe_xmlns_table_find(
		struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len)
{
	evcpe_trace(__func__, "finding namespace: %.*s", name_len, name);
	struct evcpe_xmlns ns;
	ns.name = name;
	ns.name_len = name_len;
	return RB_FIND(evcpe_xmlns_table, table, &ns);
}

int evcpe_xmlns_table_add(struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc;
	struct evcpe_xmlns *ns;

	if ((ns = evcpe_xmlns_table_find(table, name, name_len))) {
		evcpe_error(__func__, "namespace already exists: %.*s",
				name_len, name);
		rc = -1;
		goto finally;
	}

	evcpe_trace(__func__, "adding namespace: %.*s => %.*s",
			name_len, name, value_len, value);

	if (!(ns = calloc(1, sizeof(struct evcpe_xmlns)))) {
		evcpe_error(__func__, "failed to calloc evcpe_xmlns");
		rc = ENOMEM;
		goto finally;
	}
	ns->name = name;
	ns->name_len = name_len;
	ns->value = value;
	ns->value_len = value_len;
	RB_INSERT(evcpe_xmlns_table, table, ns);
	rc = 0;

finally:
	return rc;
}

int evcpe_xmlns_table_get(struct evcpe_xmlns_table *table,
		const char *name, unsigned name_len,
		const char **value, unsigned *value_len)
{
	int rc;
	struct evcpe_xmlns *ns;

	if (!(ns = evcpe_xmlns_table_find(table, name, name_len))) {
		evcpe_error(__func__, "namespace doesn't exists: %.*s",
				name_len, name);
		rc = -1;
		goto finally;
	}
	*value = ns->value;
	*value_len = ns->value_len;
	rc = 0;

finally:
	return rc;
}
