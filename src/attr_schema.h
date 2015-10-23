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
#include "plugin-priv.h"


typedef struct _evcpe_attr evcpe_attr;
typedef struct _evcpe_class evcpe_class;
typedef struct _evcpe_attr_schema evcpe_attr_schema;

typedef int (*evcpe_attr_getter_t)(evcpe_attr *attr,
		const char **value, unsigned int *len);

typedef int (*evcpe_attr_setter_t)(evcpe_attr *attr,
		const char *value, unsigned int len);

struct _evcpe_attr_schema {
	evcpe_class *owner;
	evcpe_class *class;
	char *name;
	char *value;
	evcpe_type_t type;
	unsigned extension:1;
	unsigned inform:1;
	unsigned write:1;
	unsigned notification:3; //enum evcpe_notification
	char *number;
	evcpe_constraint *constraint;
	char *pattern; // regex pattern
	evcpe_attr_getter_t getter;
	evcpe_attr_setter_t setter;
	evcpe_plugin* plugin;
};

evcpe_attr_schema *evcpe_attr_schema_new(evcpe_class *owner);

void evcpe_attr_schema_free(evcpe_attr_schema *schema);

int evcpe_attr_schema_set_name(evcpe_attr_schema *schema,
		const char *name, unsigned len);

int evcpe_attr_schema_set_type(evcpe_attr_schema *schema,
		evcpe_type_t type);

int evcpe_attr_schema_set_default(evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_number(evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_notification(evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_extension(evcpe_attr_schema *schema, int val);

int evcpe_attr_schema_set_write(evcpe_attr_schema *schema, int val);

int evcpe_attr_schema_set_constraint(evcpe_attr_schema *schema,
		const char *value, unsigned len);

int evcpe_attr_schema_set_pattern(evcpe_attr_schema *schema,
		const char *value, unsigned len);

#endif /* EVCPE_ATTR_SCHEMA_H_ */
