// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

void evcpe_method_list_init(struct evcpe_method_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_method_list_clear(struct evcpe_method_list *list)
{
	struct evcpe_method_list_item *item;

	evcpe_debug(__func__, "clearing evcpe_method_list");

	while((item = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, item, entry);
		free(item);
	}
}

unsigned int evcpe_method_list_size(struct evcpe_method_list *list)
{
	return list->size;
}

int evcpe_method_list_add(struct evcpe_method_list *list,
		struct evcpe_method_list_item **item, const char *name, unsigned len)
{
	if (len >= sizeof((*item)->name)) return EOVERFLOW;

	if (!((*item) = calloc(1, sizeof(struct evcpe_method_list_item)))) {
		evcpe_error(__func__, "failed to calloc evcpe_method_list_item");
		return ENOMEM;
	}
	strncpy((*item)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *item, entry);
	list->size ++;
	return 0;
}

void evcpe_method_list_remove(struct evcpe_method_list *list,
		struct evcpe_method_list_item *item)
{
	TAILQ_REMOVE(&list->head, item, entry);
	list->size --;
	free(item);
}

int evcpe_method_list_add_method(struct evcpe_method_list *list,
		const char *method)
{
	struct evcpe_method_list_item *item;
	return evcpe_method_list_add(list, &item,
			method, strlen(method));
}
