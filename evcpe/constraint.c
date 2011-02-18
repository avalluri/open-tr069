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
#include <string.h>
#include <errno.h>

#include "log.h"
#include "util.h"

#include "constraint.h"

int evcpe_constraint_enums_add(struct evcpe_constraint_enums *enums,
		const char *string, unsigned len)
{
	int rc;
	struct evcpe_constraint_enum *cons_enum;

	evcpe_debug(__func__, "adding enum constraint: %.*s", len, string);

	if (!(cons_enum = calloc(1, sizeof(struct evcpe_constraint_enum)))) {
		evcpe_error(__func__, "failed to calloc evcpe_constraint_enum");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_strdup(string, len, &cons_enum->string))) {
		evcpe_error(__func__, "failed to duplicate: %.*s", len, string);
		free(cons_enum);
		goto finally;
	}
	TAILQ_INSERT_TAIL(enums, cons_enum, entry);
	rc = 0;

finally:
	return rc;
}

void evcpe_constraint_enums_clear(struct evcpe_constraint_enums *enums)
{
	struct evcpe_constraint_enum *cons_enum;

	evcpe_trace(__func__, "clearing enum constraints");

	while((cons_enum = TAILQ_FIRST(enums))) {
		TAILQ_REMOVE(enums, cons_enum, entry);
		free(cons_enum->string);
		free(cons_enum);
	}
}

static inline int evcpe_constraint_set_min_max(long *ptr,
		const char *value, unsigned len)
{
	int rc;
	if ((rc = evcpe_atol(value, len, ptr))) {
		evcpe_error(__func__, "failed to convert to integer: %.*s", len, value);
	}
	return rc;
}

int evcpe_constraint_set_min(struct evcpe_constraint *cons,
		const char *min, unsigned len)
{
	evcpe_debug(__func__, "setting min constraint: %.*s", len, min);
	cons->type = EVCPE_CONSTRAINT_MIN;
	return evcpe_constraint_set_min_max(&cons->value.range.min,
			min, len);
}

int evcpe_constraint_set_max(struct evcpe_constraint *cons,
		const char *max, unsigned len)
{
	evcpe_debug(__func__, "setting max constraint: %.*s", len, max);
	cons->type = EVCPE_CONSTRAINT_MAX;
	return evcpe_constraint_set_min_max(&cons->value.range.max,
			max, len);
}

int evcpe_constraint_set_range(struct evcpe_constraint *cons,
		const char *min, unsigned minlen, const char *max, unsigned maxlen)
{
	int rc;

	evcpe_debug(__func__, "setting range constraint: %.*s:%.*s",
			minlen, min, maxlen, max);

	cons->type = EVCPE_CONSTRAINT_RANGE;
	if ((rc = evcpe_constraint_set_min_max(&cons->value.range.min,
			min, minlen))) {
		evcpe_error(__func__, "failed to set min constraint");
		goto finally;
	}
	if ((rc = evcpe_constraint_set_min_max(&cons->value.range.max,
			max, maxlen))) {
		evcpe_error(__func__, "failed to set max constraint");
		goto finally;
	}

finally:
	return rc;
}

int evcpe_constraint_set_attr(struct evcpe_constraint *cons,
		const char *value, unsigned len)
{
	evcpe_debug(__func__, "setting attribute constraint: %.*s",	len, value);
	cons->type = EVCPE_CONSTRAINT_ATTR;
	return evcpe_strdup(value, len, &cons->value.attr);
}
