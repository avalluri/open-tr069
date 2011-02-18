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

#ifndef EVCPE_CONSTRAINT_H_
#define EVCPE_CONSTRAINT_H_

#include <sys/types.h>
#include <sys/queue.h>

enum evcpe_constraint_type {
	EVCPE_CONSTRAINT_NONE,
	EVCPE_CONSTRAINT_ATTR,
	EVCPE_CONSTRAINT_SIZE,
	EVCPE_CONSTRAINT_ENUM,
	EVCPE_CONSTRAINT_MIN,
	EVCPE_CONSTRAINT_MAX,
	EVCPE_CONSTRAINT_RANGE
};

struct evcpe_constraint_enum {
	char *string;
	TAILQ_ENTRY(evcpe_constraint_enum) entry;
};

TAILQ_HEAD(evcpe_constraint_enums, evcpe_constraint_enum);

struct evcpe_constraint {
	enum evcpe_constraint_type type;
	union {
		unsigned size;
		struct evcpe_constraint_enums enums;
		struct {
			long min;
			long max;
		} range;
		char *attr;
	} value;
};

int evcpe_constraint_enums_add(struct evcpe_constraint_enums *enums,
		const char *string, unsigned len);

void evcpe_constraint_enums_clear(struct evcpe_constraint_enums *enums);

int evcpe_constraint_set_min(struct evcpe_constraint *cons,
		const char *min, unsigned len);

int evcpe_constraint_set_max(struct evcpe_constraint *cons,
		const char *max, unsigned len);

int evcpe_constraint_set_range(struct evcpe_constraint *cons,
		const char *min, unsigned minlen, const char *max, unsigned maxlen);

int evcpe_constraint_set_attr(struct evcpe_constraint *cons,
		const char *value, size_t len);

#endif /* EVCPE_CONSTRAINT_H_ */
