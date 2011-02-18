// $Id$
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
