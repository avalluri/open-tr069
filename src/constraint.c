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

static
int evcpe_constraint_enums_add(struct evcpe_constraint_enums *enums,
		const char *string, unsigned len)
{
	int rc;
	struct evcpe_constraint_enum *cons_enum;

	DEBUG("adding enum constraint: %.*s", len, string);

	if (!(cons_enum = calloc(1, sizeof(struct evcpe_constraint_enum)))) {
		ERROR("failed to calloc evcpe_constraint_enum");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_strdup(string, len, &cons_enum->string))) {
		ERROR("failed to duplicate: %.*s", len, string);
		free(cons_enum);
		goto finally;
	}
	TAILQ_INSERT_TAIL(enums, cons_enum, entry);
	rc = 0;

finally:
	return rc;
}

static
void evcpe_constraint_enums_clear(struct evcpe_constraint_enums *enums)
{
	struct evcpe_constraint_enum *cons_enum;

	TRACE("clearing enum constraints");

	while((cons_enum = TAILQ_FIRST(enums))) {
		TAILQ_REMOVE(enums, cons_enum, entry);
		free(cons_enum->string);
		free(cons_enum);
	}
}

int evcpe_constraint_set_enums(struct evcpe_constraint *cons,
		const char *str_enums, unsigned len)
{
	int rc = 0;
	const char *start = NULL, *end = NULL;

	TAILQ_INIT(&cons->value.enums);
	for (start = end = str_enums; end < str_enums + len; end++) {
		if (*end == '|') {
			if ((rc = evcpe_constraint_enums_add(&cons->value.enums, start,
					end - start))) {
				ERROR("failed to add enum constraint");
				goto finally;
			}
			start = end + 1;
		}
	}
	if ((rc = evcpe_constraint_enums_add(&cons->value.enums, start,
			end - start))) {
		ERROR("failed to add enum constraint");
		goto finally;
	}
	cons->type = EVCPE_CONSTRAINT_ENUM;

finally:
	return rc;
}

int evcpe_constraint_get_enums(struct evcpe_constraint *cons,
		struct evcpe_constraint_enums **enums_out)
{
	if (!cons || !enums_out) return EPROTO;
	if (cons->type != EVCPE_CONSTRAINT_ENUM) return EINVAL;

	*enums_out = &cons->value.enums;

	return 0;
}

static inline
int evcpe_constraint_set_min_max(long *ptr,
		const char *value, unsigned len)
{
	int rc = 0;
	if ((rc = evcpe_atol(value, len, ptr))) {
		ERROR("failed to convert to integer: %.*s", len, value);
	}
	return rc;
}

int evcpe_constraint_set_min(struct evcpe_constraint *cons,
		const char *min, unsigned len)
{
	DEBUG("setting min constraint: %.*s", len, min);
	cons->type = EVCPE_CONSTRAINT_MIN;
	return evcpe_constraint_set_min_max(&cons->value.range.min,
			min, len);
}

int evcpe_constraint_get_min(struct evcpe_constraint *cons, long *min_out)
{
	if(!cons || !min_out) return EPROTO;
	if(cons->type != EVCPE_CONSTRAINT_MIN) return EINVAL;

	*min_out = cons->value.range.min;

	return 0;
}

int evcpe_constraint_set_max(struct evcpe_constraint *cons,
		const char *max, unsigned len)
{
	DEBUG("setting max constraint: %.*s", len, max);
	cons->type = EVCPE_CONSTRAINT_MAX;
	return evcpe_constraint_set_min_max(&cons->value.range.max, max, len);
}

int evcpe_constraint_get_max(struct evcpe_constraint *cons, long *max_out)
{
	if(!cons || !max_out) return EPROTO;
	if(cons->type != EVCPE_CONSTRAINT_MAX) return EINVAL;

	*max_out = cons->value.range.max;

	return 0;
}

int evcpe_constraint_set_range(struct evcpe_constraint *cons,
		const char *min, unsigned minlen, const char *max, unsigned maxlen)
{
	int rc;

	DEBUG("setting range constraint: %.*s:%.*s", minlen, min, maxlen, max);

	cons->type = EVCPE_CONSTRAINT_RANGE;
	if ((rc = evcpe_constraint_set_min_max(&cons->value.range.min,
			min, minlen))) {
		ERROR("failed to set min constraint");
		goto finally;
	}
	if ((rc = evcpe_constraint_set_min_max(&cons->value.range.max, max,
			maxlen))) {
		ERROR("failed to set max constraint");
		goto finally;
	}

finally:
	return rc;
}

int evcpe_constraint_get_range(struct evcpe_constraint *cons, long *min_out,
		long *max_out)
{
	if(!cons || !min_out || !max_out) return EPROTO;
	if(cons->type != EVCPE_CONSTRAINT_RANGE) return EINVAL;

	*min_out = cons->value.range.min;
	*max_out = cons->value.range.max;

	return 0;
}

int evcpe_constraint_set_size(struct evcpe_constraint *cons, long size)
{
	DEBUG("setting size constraint %ld", size);

	if (!cons) return EPROTO;

	cons->type = EVCPE_CONSTRAINT_SIZE;
	cons->value.size = size;

	return 0;
}

int evcpe_constraint_get_size(struct evcpe_constraint *cons, long *size_out)
{
	if (!cons || !size_out) return EPROTO;
	if (cons->type != EVCPE_CONSTRAINT_SIZE) return EINVAL;

	*size_out = cons->value.size;
	return 0;
}

int evcpe_constraint_set_attr(struct evcpe_constraint *cons,
		const char *value, unsigned len)
{
	DEBUG("setting attribute constraint: %.*s",	len, value);
	cons->type = EVCPE_CONSTRAINT_ATTR;
	return evcpe_strdup(value, len, &cons->value.attr);
}

int evcpe_constraint_get_attr(struct evcpe_constraint *cons, char **attr_out)
{
	if (!cons || attr_out) return EPROTO;
	if (cons->type != EVCPE_CONSTRAINT_ATTR) return EINVAL;
	if (!cons->value.attr) return 0;

	*attr_out = strdup(cons->value.attr);
	if (!*attr_out) return ENOMEM;

	return 0;
}

struct evcpe_constraint *evcpe_constraint_new()
{
	struct evcpe_constraint *self = NULL;

	if (!(self = (struct evcpe_constraint *)
			malloc(sizeof(struct evcpe_constraint)))) {
		return NULL;
	}

	self->type = EVCPE_CONSTRAINT_NONE;

	return self;
}

void evcpe_constraint_free(struct evcpe_constraint *cons)
{
	if (!cons) return;

	if (cons->type == EVCPE_CONSTRAINT_ENUM)
		evcpe_constraint_enums_clear(&cons->value.enums);
	else if (cons->type == EVCPE_CONSTRAINT_ATTR)
		free(cons->value.attr);

	free(cons);
}
