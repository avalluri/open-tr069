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

#ifndef EVCPE_OBJ_H_
#define EVCPE_OBJ_H_

#include <sys/types.h>

#include "attr.h"

typedef struct _evcpe_obj {
	char *path;
	unsigned int pathlen;
	evcpe_class *class;
	evcpe_attr *owner;
	unsigned int index;
	evcpe_attrs attrs;
} evcpe_obj;

evcpe_obj *evcpe_obj_new(evcpe_class *class,
		evcpe_attr *owner);

void evcpe_obj_free(evcpe_obj *obj);

int evcpe_obj_init(evcpe_obj *obj);

evcpe_attr* evcpe_obj_find(evcpe_obj *obj,
		evcpe_attr_schema *schema);

evcpe_attr* evcpe_obj_find_deep(evcpe_obj *obj,
		evcpe_attr_schema *schema);

int evcpe_obj_get(evcpe_obj *obj,
		const char *name, unsigned len, evcpe_attr **attr);

int evcpe_obj_get_attr_value(evcpe_obj* obj, const char* attr_name,
		const char** value, unsigned* value_len);

#endif /* EVCPE_OBJ_H_ */
