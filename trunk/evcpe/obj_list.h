// $Id$

#ifndef EVCPE_OBJ_LIST_H_
#define EVCPE_OBJ_LIST_H_

#include <sys/queue.h>

struct evcpe_obj;

struct evcpe_obj_item {
	struct evcpe_obj *obj;
	TAILQ_ENTRY(evcpe_obj_item) entry;
};

TAILQ_HEAD(evcpe_obj_list, evcpe_obj_item);

struct evcpe_obj_item *evcpe_obj_list_get(
		struct evcpe_obj_list *list, int index);

void evcpe_obj_list_clear(struct evcpe_obj_list *list);

#endif /* EVCPE_OBJ_LIST_H_ */
