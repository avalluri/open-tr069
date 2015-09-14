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

#include <stdio.h>
#include <stdlib.h>

#include "obj.h"

#include "obj_list.h"

void evcpe_obj_list_clear(struct evcpe_obj_list *list)
{
	struct evcpe_obj_item *item;

	while((item = TAILQ_FIRST(list))) {
		if (item->obj) {
			evcpe_obj_free(item->obj);
		}
		TAILQ_REMOVE(list, item, entry);
		free(item);
	}
}

struct evcpe_obj_item *evcpe_obj_list_get(
		struct evcpe_obj_list *list, int index)
{
	struct evcpe_obj_item *item;
	int i;

	i = 0;
	TAILQ_FOREACH(item, list, entry) {
		if (i == index) {
			return item;
			break;
		}
		i ++;
	}
	return NULL;
}
