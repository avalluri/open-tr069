// $Id$

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "data.h"

void evcpe_access_list_init(struct evcpe_access_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void evcpe_access_list_clear(struct evcpe_access_list *list)
{
	struct evcpe_access_list_item *item;

	evcpe_trace(__func__, "clearing evcpe_access_list");

	while((item = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, item, entry);
		free(item);
	}
}

int evcpe_access_list_clone(struct evcpe_access_list *src,
		struct evcpe_access_list *dst)
{
	int rc;
	struct evcpe_access_list_item *src_item, *dst_item;

	evcpe_debug(__func__, "cloning evcpe_access_list");

	TAILQ_FOREACH(src_item, &src->head, entry) {
		if ((rc = evcpe_access_list_add(dst, &dst_item,
				src_item->entity, strlen(src_item->entity)))) {
			evcpe_error(__func__, "failed to add entity to destination list");
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

unsigned int evcpe_access_list_size(struct evcpe_access_list *list)
{
	return list->size;
}

int evcpe_access_list_add(struct evcpe_access_list *list,
		struct evcpe_access_list_item **item, const char *entity, unsigned len)
{
	if (len >= sizeof((*item)->entity)) return EOVERFLOW;

	evcpe_trace(__func__, "adding entity: %.*s", len, entity);

	if (!((*item) = calloc(1, sizeof(struct evcpe_access_list_item)))) {
		return ENOMEM;
	}
	strncpy((*item)->entity, entity, len);
	TAILQ_INSERT_TAIL(&list->head, *item, entry);
	list->size ++;
	return 0;
}

void evcpe_access_list_remove(struct evcpe_access_list *list,
		struct evcpe_access_list_item *item)
{
	TAILQ_REMOVE(&list->head, item, entry);
	list->size --;
	free(item);
}
