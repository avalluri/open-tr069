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

static
int _find_schema_by_name(evcpe_attr_schema *schema, const char *name)
{
	return evcpe_strncmp(schema->name, name, strlen(name));
}

evcpe_class *evcpe_class_new(const char *name)
{
	evcpe_class *class = NULL;

	TRACE("constructing evcpe_class: %s", name);

	if (!(class = calloc(1, sizeof(evcpe_class)))) {
		ERROR("failed to calloc evcpe_class");
		return NULL;
	}
	class->name = name;
	class->attrs = tqueue_new((tqueue_compare_func_t)_find_schema_by_name,
			(tqueue_free_func_t)evcpe_attr_schema_free);
	/* Inform attributes are cached only in root class */
	if (!name) class->inform_attrs =  tqueue_new(NULL, NULL);

	return class;
}

void evcpe_class_free(evcpe_class *class)
{
	if (!class) return;

	TRACE("destructing evcpe_class: %s", class->name);

	tqueue_free(class->attrs);
	if (class->inform_attrs) tqueue_free(class->inform_attrs);

	free(class);
}

int evcpe_class_add_new_schema(evcpe_class *class,
		evcpe_attr_schema **schema)
{
	TRACE("adding attribute to class: %s\n", class->name);

	if (!(*schema = evcpe_attr_schema_new(class))) return ENOMEM;

	tqueue_insert(class->attrs, *schema);

	return 0;
}


evcpe_attr_schema *evcpe_class_find(evcpe_class *class,
		const char *name, unsigned len)
{
	char str_name[256];
	tqueue_element *elm = NULL;

	if (!class || !name || !len) return NULL;

	if (len > 255) len = 255;
	snprintf(str_name, len+1, "%s", name);

	elm = tqueue_find(class->attrs, str_name);
	return elm ? elm->data : NULL;
}
