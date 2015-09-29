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

#ifndef EVCPE_ATTR_SCHEMA_H_
#define EVCPE_ATTR_SCHEMA_H_

//#include <sys/queue.h>

#include "type.h"
#include "data.h"
#include "plugin.h"

struct evcpe_class;

struct evcpe_attr;

typedef int (*evcpe_attr_getter)(struct evcpe_attr *attr,
		const char **value, unsigned int *len);

typedef int (*evcpe_attr_setter)(struct evcpe_attr *attr,
		const char *value, unsigned int len);

struct evcpe_attr_schema {
	struct evcpe_class *owner;
	struct evcpe_class *class;
	char *name;
	char *value;
	enum evcpe_type type;
	unsigned extension:1;
	unsigned inform:1;
	unsigned write:1;
	unsigned notification:3; //enum evcpe_notification
	char *number;
	struct evcpe_constraint *constraint;
	char *pattern; // regex pattern
	evcpe_attr_getter getter;
	evcpe_attr_setter setter;
	evcpe_plugin* plugin;
};

struct evcpe_attr_schema *evcpe_attr_schema_new(struct evcpe_class *owner);

void evcpe_attr_schema_free(struct evcpe_attr_schema *schema);

int evcpe_attr_schema_set_name(struct evcpe_attr_schema *schema,
		const char *name, unsigned len);

int evcpe_attr_schema_set_type(struct evcpe_attr_schema *schema,
		enum evcpe_type type);

int evcpe_attr_schema_set_default(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_number(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_notification(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_extension(struct evcpe_attr_schema *schema, int val);

int evcpe_attr_schema_set_write(struct evcpe_attr_schema *schema, int val);

int evcpe_attr_schema_set_constraint(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_pattern(struct evcpe_attr_schema *schema,
		const char *value, unsigned len);

#endif /* EVCPE_ATTR_SCHEMA_H_ */
