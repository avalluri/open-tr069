// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

void evcpe_set_param_value_list_init(struct evcpe_set_param_value_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_set_param_value_list_clear(struct evcpe_set_param_value_list *list)
{
	struct evcpe_set_param_value *value;

	evcpe_debug(__func__, "clearing evcpe_set_param_value_list");

	while((value = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, value, entry);
		free(value);
	}
}

unsigned int evcpe_set_param_value_list_size(
		struct evcpe_set_param_value_list *list)
{
	return list->size;
}

int evcpe_set_param_value_list_add(struct evcpe_set_param_value_list *list,
		struct evcpe_set_param_value **value, const char *name, unsigned len)
{
	if (!name || !len) return EINVAL;
	if (len >= sizeof((*value)->name)) return EOVERFLOW;

	if (!(*value = calloc(1, sizeof(struct evcpe_set_param_value))))
		return ENOMEM;
	strncpy((*value)->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, *value, entry);
	list->size ++;
	return 0;
}

int evcpe_set_param_value_set(struct evcpe_set_param_value *param,
		const char *data, unsigned len)
{
	if (!param || !data) return EINVAL;

	evcpe_debug(__func__, "setting value: %s => %.*s",
			param->name, len, data);
	param->data = data;
	param->len = len;
	return 0;
}
