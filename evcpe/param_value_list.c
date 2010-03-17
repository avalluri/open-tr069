// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "obj.h"

#include "data.h"

void evcpe_param_value_list_init(struct evcpe_param_value_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_param_value_list_clear(struct evcpe_param_value_list *list)
{
	struct evcpe_param_value *param;

	evcpe_debug(__func__, "clearing evcpe_param_value_list");

	while((param = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, param, entry);
		free(param);
	}
	list->size = 0;
}

int evcpe_param_value_set(struct evcpe_param_value *param,
		const char *data, unsigned len)
{
	if (!param) return EINVAL;

	evcpe_debug(__func__, "setting value: %s => %.*s",
			param->name, len, data);
	param->data = data;
	param->len = len;
	return 0;
}

unsigned int evcpe_param_value_list_size(struct evcpe_param_value_list *list)
{
	return list->size;
}

int evcpe_param_value_list_add(struct evcpe_param_value_list *list,
		struct evcpe_param_value **value, const char *name, unsigned len)
{
	int rc;
	struct evcpe_param_value *param;

	if (!name || !len) return EINVAL;
	if (len >= sizeof(param->name)) return EOVERFLOW;

	evcpe_debug(__func__, "adding parameter: %.*s", len, name);

	if (!(param = calloc(1, sizeof(struct evcpe_param_value)))) {
		evcpe_error(__func__, "failed to calloc evcpe_param_value");
		rc = ENOMEM;
		goto finally;
	}
	strncpy(param->name, name, len);
	TAILQ_INSERT_TAIL(&list->head, param, entry);
	*value = param;
	list->size ++;
	rc = 0;

finally:
	return rc;
}

void evcpe_param_value_list_remove(struct evcpe_param_value_list *list,
		struct evcpe_param_value *param)
{
	TAILQ_REMOVE(&list->head, param, entry);
	list->size --;
	free(param);
}
