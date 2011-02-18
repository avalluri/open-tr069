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

#ifndef EVCPE_CLASS_H_
#define EVCPE_CLASS_H_

#include "attr_schema.h"

TAILQ_HEAD(evcpe_attr_schemas, evcpe_attr_schema);

struct evcpe_class {
	const char *name;
	struct evcpe_attr_schemas attrs;
};

struct evcpe_class *evcpe_class_new(const char *name);

void evcpe_class_free(struct evcpe_class *class);

struct evcpe_attr_schema *evcpe_class_find(struct evcpe_class *class,
		const char *name, unsigned len);

int evcpe_class_add(struct evcpe_class *class,
		struct evcpe_attr_schema **schema);

int evcpe_class_add_attr(struct evcpe_class *class, const char *name,
		enum evcpe_type type, struct evcpe_attr_schema **schema);

#endif /* EVCPE_CLASS_H_ */
