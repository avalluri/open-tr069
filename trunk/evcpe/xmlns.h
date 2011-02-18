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
