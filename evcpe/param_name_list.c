// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

void evcpe_param_name_list_init(struct evcpe_param_name_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_param_name_list_clear(struct evcpe_param_name_list *list)
{
	struct evcpe_param_name *name;

	evcpe_debug(__func__, "clearing evcpe_param_name_list");

	while((name = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, name, entry);
		free(name);
	}
	list->size = 0;
}

unsigned int evcpe_param_name_list_size(struct evcpe_param_name_list *list)
{
	return list->size;
}

int evcpe_param_name_list_add(struct evcpe_param_name_list *list,
		struct evcpe_param_name **param, const char *name, unsigned len)
{
	struct evcpe_param_name *param_name;

	if (!name || !len) return EINVAL;
	if (len >= sizeof(param_name->name)) return EOVERFLOW;

	evcpe_debug(__func__, "adding param name: %.*s", len, name);

	if (!(param_name = calloc(1, sizeof(struct evcpe_param_name))))
		return ENOMEM;

	strncpy(param_name->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, param_name, entry);
	list->size ++;
	*param = param_name;
	return 0;
}

void evcpe_param_name_list_remove(struct evcpe_param_name_list *list,
		struct evcpe_param_name *param)
{
	TAILQ_REMOVE(&list->head, param, entry);
	list->size --;
	free(param);
}
