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
#include "tqueue.h"

enum evcpe_constraint_type {
	EVCPE_CONSTRAINT_NONE,
	EVCPE_CONSTRAINT_ATTR,
	EVCPE_CONSTRAINT_SIZE,
	EVCPE_CONSTRAINT_ENUM,
	EVCPE_CONSTRAINT_MIN,
	EVCPE_CONSTRAINT_MAX,
	EVCPE_CONSTRAINT_RANGE
};

typedef struct _evcpe_constraint {
	enum evcpe_constraint_type type;
	union {
		unsigned size;
		tqueue* enums;
		struct {
			long min;
			long max;
		} range;
		char *attr;
	} value;
} evcpe_constraint;

evcpe_constraint *evcpe_constraint_new();

void evcpe_constraint_free();

int evcpe_constraint_set_size(evcpe_constraint *cons, long size);

int evcpe_constraint_get_size(evcpe_constraint *cons, long *size_out);

int evcpe_constraint_set_enums(evcpe_constraint *cons,
		const char *str_enums, unsigned len);

int evcpe_constraint_get_enums(evcpe_constraint *cons, tqueue **enums_out);

int evcpe_constraint_set_min(evcpe_constraint *cons,
		const char *min, unsigned len);

int evcpe_constraint_get_min(evcpe_constraint *cons, long *min_out);

int evcpe_constraint_set_max(evcpe_constraint *cons,
		const char *max, unsigned len);

int evcpe_constraint_get_max(evcpe_constraint *cons, long *max_out);

int evcpe_constraint_set_range(evcpe_constraint *cons,
		const char *min, unsigned minlen, const char *max, unsigned maxlen);

int evcpe_constraint_get_range(evcpe_constraint *cons, long *min_out,
		long *max_out);

int evcpe_constraint_set_attr(evcpe_constraint *cons,
		const char *value, unsigned len);

int evcpe_constraint_get_attr(evcpe_constraint *cons, char **attr_out);


#endif /* EVCPE_CONSTRAINT_H_ */
