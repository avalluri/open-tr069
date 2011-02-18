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

struct evcpe_obj {
	char *path;
	unsigned int pathlen;
	struct evcpe_class *class;
	struct evcpe_attr *owner;
	unsigned int index;
	struct evcpe_attrs attrs;
};

struct evcpe_obj *evcpe_obj_new(struct evcpe_class *class,
		struct evcpe_attr *owner);

void evcpe_obj_free(struct evcpe_obj *obj);

int evcpe_obj_init(struct evcpe_obj *obj);

struct evcpe_attr *evcpe_obj_find(struct evcpe_obj *obj,
		const char *name, unsigned len);

int evcpe_obj_get(struct evcpe_obj *obj,
		const char *name, unsigned len, struct evcpe_attr **attr);

#endif /* EVCPE_OBJ_H_ */
