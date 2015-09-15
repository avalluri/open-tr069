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

#include "class.h"

struct evcpe_class *evcpe_class_new(const char *name)
{
	struct evcpe_class *class = NULL;

	TRACE("constructing evcpe_class: %s", name);

	if (!(class = calloc(1, sizeof(struct evcpe_class)))) {
		ERROR("failed to calloc evcpe_class");
		return NULL;
	}
	class->name = name;
	TAILQ_INIT(&class->attrs);
	return class;
}

void evcpe_class_free(struct evcpe_class *class)
{
	struct evcpe_attr_schema *schema;

	if (!class) return;

	TRACE("destructing evcpe_class: %s", class->name);

	while((schema = TAILQ_FIRST(&class->attrs))) {
		TAILQ_REMOVE(&class->attrs, schema, entry);
		evcpe_attr_schema_free(schema);
	}
	free(class);
}

int evcpe_class_add_new_schema(struct evcpe_class *class,
		struct evcpe_attr_schema **schema)
{
	TRACE("adding attribute to class: %s", class->name);

	if (!(*schema = evcpe_attr_schema_new(class))) return ENOMEM;

	TAILQ_INSERT_TAIL(&class->attrs, *schema, entry);

	return 0;
}

struct evcpe_attr_schema *evcpe_class_find(struct evcpe_class *class,
		const char *name, unsigned len)
{
	struct evcpe_attr_schema *schema = NULL;

	if (!class) return NULL;

	TAILQ_FOREACH(schema, &class->attrs, entry) {
		if (!evcpe_strncmp(schema->name, name, len))
			return schema;
	}

	return NULL;
}
