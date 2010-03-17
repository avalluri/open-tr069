// $Id$

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
