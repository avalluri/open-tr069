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

#include "data.h"

void evcpe_set_param_attr_list_init(struct evcpe_set_param_attr_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_set_param_attr_list_clear(struct evcpe_set_param_attr_list *list)
{
	struct evcpe_set_param_attr *attr;

	evcpe_debug(__func__, "clearing evcpe_set_param_attr_list");

	while((attr = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, attr, entry);
		evcpe_access_list_clear(&attr->access_list);
		free(attr);
	}
}

unsigned int evcpe_set_param_attr_list_size(
		struct evcpe_set_param_attr_list *list)
{
	return list->size;
}

int evcpe_set_param_attr_list_add(struct evcpe_set_param_attr_list *list,
		struct evcpe_set_param_attr **attr, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*attr)->name)) return EOVERFLOW;

	if (!(*attr = calloc(1, sizeof(struct evcpe_set_param_attr))))
		return ENOMEM;
	strncpy((*attr)->name, name, len);
	evcpe_access_list_init(&(*attr)->access_list);
	TAILQ_INSERT_TAIL(&list->head, *attr, entry);
	list->size ++;
	return 0;
}
