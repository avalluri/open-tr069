// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

void evcpe_param_attr_list_init(struct evcpe_param_attr_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_param_attr_list_clear(struct evcpe_param_attr_list *list)
{
	struct evcpe_param_attr *attr;

	evcpe_debug(__func__, "clearing evcpe_param_attr_list");

	while((attr = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, attr, entry);
		evcpe_access_list_clear(&attr->access_list);
		free(attr);
	}
}

unsigned int evcpe_param_attr_list_size(
		struct evcpe_param_attr_list *list)
{
	return list->size;
}

int evcpe_param_attr_list_add(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr **attr, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*attr)->name)) return EOVERFLOW;

	evcpe_debug(__func__, "adding param attr: %.*s", len, name);

	if (!(*attr = calloc(1, sizeof(struct evcpe_param_attr))))
		return ENOMEM;
	evcpe_access_list_init(&(*attr)->access_list);
	strncpy((*attr)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *attr, entry);
	list->size ++;
	return 0;
}

void evcpe_param_attr_list_remove(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr *attr)
{
	TAILQ_REMOVE(&list->head, attr, entry);
	list->size --;
	evcpe_access_list_clear(&attr->access_list);
	free(attr);
}
