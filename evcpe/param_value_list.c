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
