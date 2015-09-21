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

#ifndef EVCPE_DATA_H_
#define EVCPE_DATA_H_

#include <sys/types.h>
#include <sys/queue.h>
#include <stdlib.h>

#include "type.h"

#define QLIST_DEFINE(type) \
TAILQ_HEAD(type##_head, type); \
struct type##_list { \
	struct type##_head head; \
	unsigned int size; \
}

#define QLIST_INIT(list) { \
	TAILQ_INIT(&(list)->head); \
	(list)->size = 0; \
}

#define QLIST_SIZE(list) (list)->size

#define QLIST_CLEAR(list, item_type, item_free_func) \
{ \
	item_type *item = NULL; \
 	while((item = TAILQ_FIRST(&(list)->head))) { \
  		TAILQ_REMOVE(&(list)->head, item, entry); \
		item_free_func(item); \
	} \
	(list)->size = 0; \
}

#define QLIST_REMOVE(list, item, item_free_func) \
{ \
	TAILQ_REMOVE(&(list)->head, item, entry); \
	item_free_func(item); \
	(list)->size--; \
}

#define QLIST_CLONE(src, dst, item_type, item_clone_func, rc_out) {\
	item_type *src_item = NULL; \
	item_type *dst_item = NULL; \
	int rc = 0; \
	TAILQ_FOREACH(src_item, &(src)->head, entry) { \
		if ((rc = item_clone_func(src_item, &dst_item)) != 0) break; \
		TAILQ_INSERT_TAIL(&(dst)->head, dst_item, entry); \
		(dst)->size++; \
	} \
	if (rc_out) *rc_out = rc;\
}

struct evcpe_device_id {
	char manufacturer[65];
	char oui[7];
	char product_class[65];
	char serial_number[65];
};

enum evcpe_event_code_type {
  EVCPE_EVENT_0_BOOTSTRAP,
  EVCPE_EVENT_1_BOOT,
  EVCPE_EVENT_2_PERIODIC,
  EVCPE_EVENT_3_SCHEDULED,
  EVCPE_EVENT_4_VALUE_CHANGE,
  EVCPE_EVENT_5_KICKED,
  EVCPE_EVENT_6_CONNECTION_REQUEST,
  EVCPE_EVENT_7_TRANSFER_COMPLETE,
  EVCPE_EVENT_8_DIAGNOSTICS_COMPLETE,
  EVCPE_EVENT_9_REQUEST_DOWNLOAD,
  EVCPE_EVENT_10_AUTONOMOUS_TRANSFER_COMPLETE,
  EVCPE_EVENT_11_DU_STATE_CHANGE_COMPLETE,
  EVCPE_EVENT_12_AUTONOMOUS_DU_STATE_CHANGE_COMPLETE,
  EVCPE_EVENT_13_WAKEUP,
  EVCPE_EVENT_MAX,
};

struct evcpe_event {
  enum evcpe_event_code_type event_code_type;
	char event_code[65];
	char command_key[33];
	TAILQ_ENTRY(evcpe_event) entry;
};

struct evcpe_event* evcpe_event_new();
int evcpe_event_clone(struct evcpe_event *src, struct evcpe_event **dst);
const char *evcpe_event_code_type_to_str(enum evcpe_event_code_type type);

QLIST_DEFINE(evcpe_event);

#define evcpe_event_list_init(list) QLIST_INIT(list)
#define evcpe_event_list_clear(list) \
	QLIST_CLEAR((list), struct evcpe_event, free)
#define evcpe_event_list_size(list) (list)->size
#define evcpe_event_list_remove(list, event) QLIST_REMOVE(list, item, free)
#define evcpe_event_list_clone(src, dst, rc_out) \
	QLIST_CLONE(src, dst, struct evcpe_event, evcpe_event_clone, rc_out)

int evcpe_event_list_add(struct evcpe_event_list *list,
		struct evcpe_event **event,
		const char *event_code, const char *command_key);

void evcpe_event_list_remove_event(struct evcpe_event_list *list,
		const char *event_code);


struct evcpe_param_info {
	char name[257];
	int writable;
	TAILQ_ENTRY(evcpe_param_info) entry;
};

struct evcpe_param_info* evcpe_param_info_new();
int evcpe_param_info_clone(struct evcpe_param_info *src,
		struct evcpe_param_info **dst);

QLIST_DEFINE(evcpe_param_info);
/*TAILQ_HEAD(evcpe_param_info_head, evcpe_param_info);

struct evcpe_param_info_list {
	struct evcpe_param_info_head head;
	unsigned int size;
};
*/
#define evcpe_param_info_list_init(list) QLIST_INIT(list)
#define evcpe_param_info_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_param_info, free)
#define evcpe_param_info_list_size(list) QLIST_SIZE(list)
#define evcpe_param_info_list_remove(list, info) QLIST_REMOVE(list, info, free)

int evcpe_param_info_list_add(struct evcpe_param_info_list *list,
		struct evcpe_param_info **param, const char *name, unsigned len,
		int writable);


struct evcpe_param_name {
	char name[257];
	TAILQ_ENTRY(evcpe_param_name) entry;
};

struct evcpe_param_name* evcpe_param_name_new();

QLIST_DEFINE(evcpe_param_name);
/*
TAILQ_HEAD(evcpe_param_name_head, evcpe_param_name);

struct evcpe_param_name_list {
	struct evcpe_param_name_head head;
	unsigned int size;
};
*/

#define evcpe_param_name_list_init(list) QLIST_INIT(list)
#define evcpe_param_name_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_param_name, free)
#define evcpe_param_name_list_size(list) QLIST_SIZE(list)
#define evcpe_param_name_list_remove(list, item) QLIST_REMOVE(list, item, free)

int evcpe_param_name_list_add(struct evcpe_param_name_list *list,
		struct evcpe_param_name **param, const char *name, unsigned len);

struct evcpe_param_value {
	char name[257];
	enum evcpe_type type;
	const char *data;
	unsigned len;
	TAILQ_ENTRY(evcpe_param_value) entry;
};

int evcpe_param_value_set(struct evcpe_param_value *value,
		const char *data, unsigned len);

QLIST_DEFINE(evcpe_param_value);

#define evcpe_param_value_list_init(list) QLIST_INIT(list)
#define evcpe_param_value_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_param_value, free)
#define evcpe_param_value_list_size(list) QLIST_SIZE(list)
#define evcpe_param_value_list_remove(list, value) \
	QLIST_REMOVE(list, value, free)

int evcpe_param_value_list_add(struct evcpe_param_value_list *list,
		struct evcpe_param_value **value, const char *name, unsigned len);

int evcpe_param_value_list_add_value(struct evcpe_param_value_list *list,
		const char *name, enum evcpe_type type, const char *value);

struct evcpe_access {
	char entity[65];
	TAILQ_ENTRY(evcpe_access) entry;
};

struct evcpe_access * evcpe_access_new();

int evcpe_access_clone(struct evcpe_access *src, struct evcpe_access **dst);

QLIST_DEFINE(evcpe_access);

#define evcpe_access_list_init(list) QLIST_INIT(list)
#define evcpe_access_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_access, free)
#define evcpe_access_list_size(list) QLIST_SIZE(list)
#define evcpe_access_list_clone(src, dst, rc_out) \
	QLIST_CLONE(src, dst, struct evcpe_access,\
					evcpe_access_clone, rc_out)

int evcpe_access_list_add(struct evcpe_access_list *list,
		const char *entity, unsigned len);

enum evcpe_notification {
	EVCPE_NOTIFICATION_OFF,
	EVCPE_NOTIFICATION_PASSIVE,
	EVCPE_NOTIFICATION_ACTIVE
};

struct evcpe_param_attr {
	char name[257];
	enum evcpe_notification notification;
	struct evcpe_access_list access_list;
	TAILQ_ENTRY(evcpe_param_attr) entry;
};

QLIST_DEFINE(evcpe_param_attr);

#define evcpe_param_attr_list_init(list) QLIST_INIT(list)
#define evcpe_param_attr_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_param_attr, free)
#define evcpe_param_attr_list_size(list) QLIST_SIZE(list)
#define evcpe_param_attr_list_remove(list, attr) \
	QLIST_REMOVE(list, item, free)

int evcpe_param_attr_list_add(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr **attr, const char *name, unsigned len);

struct evcpe_set_param_attr {
	char name[257];
	int notification_change;
	enum evcpe_notification notification;
	int access_list_change;
	struct evcpe_access_list access_list;
	TAILQ_ENTRY(evcpe_set_param_attr) entry;
};

struct evcpe_set_param_attr* evcpe_set_param_attr_new();

QLIST_DEFINE(evcpe_set_param_attr);

#define evcpe_set_param_attr_list_init(list) QLIST_INIT(list)
#define evcpe_set_param_attr_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_set_param_attr, free)
#define evcpe_set_param_attr_list_size(list) QLIST_SIZE(list)
#define evcpe_set_param_attr_list_remove(list, attr) \
	QLIST_REMOVE(list, attr, free)

int evcpe_set_param_attr_list_add(struct evcpe_set_param_attr_list *list,
		struct evcpe_set_param_attr **attr, const char *name, unsigned len);

struct evcpe_set_param_value {
	char name[257];
	const char *data;
	unsigned int len;
	TAILQ_ENTRY(evcpe_set_param_value) entry;
};

int evcpe_set_param_value_set(struct evcpe_set_param_value *param,
		const char *data, unsigned len);

QLIST_DEFINE(evcpe_set_param_value);

#define evcpe_set_param_value_list_init(list) QLIST_INIT(list)
#define evcpe_set_param_value_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_set_param_value, free)
#define evcpe_set_param_value_list_size(list) QLIST_SIZE(list)

int evcpe_set_param_value_list_add(struct evcpe_set_param_value_list *list,
		struct evcpe_set_param_value **value, const char *name, unsigned len);

struct evcpe_method {
	char name[65];
	TAILQ_ENTRY(evcpe_method) entry;
};

QLIST_DEFINE(evcpe_method);

#define evcpe_method_list_init(list) QLIST_INIT(list)
#define evcpe_method_list_clear(list) \
	QLIST_CLEAR(list, struct evcpe_method, free)
#define evcpe_method_list_size(list) QLIST_SIZE(list)
#define evcpe_method_list_remove(list, method) QLIST_REMOVE(list, method, free)

int evcpe_method_list_add(struct evcpe_method_list *list,
		struct evcpe_method **item, const char *name, unsigned len);

int evcpe_method_list_add_method(struct evcpe_method_list *list,
		const char *method);

#endif /* EVCPE_DATA_H_ */
