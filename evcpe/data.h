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

#include "type.h"

struct evcpe_device_id {
	char manufacturer[65];
	char oui[7];
	char product_class[65];
	char serial_number[65];
};

struct evcpe_event {
	char event_code[65];
	char command_key[33];
	TAILQ_ENTRY(evcpe_event) entry;
};

TAILQ_HEAD(evcpe_event_head, evcpe_event);

struct evcpe_event_list {
	struct evcpe_event_head head;
	unsigned int size;
};

inline void evcpe_event_list_init(struct evcpe_event_list *list);

void evcpe_event_list_clear(struct evcpe_event_list *list);

inline unsigned int evcpe_event_list_size(struct evcpe_event_list *list);

int evcpe_event_list_clone(struct evcpe_event_list *src,
		struct evcpe_event_list *dst);

int evcpe_event_list_add(struct evcpe_event_list *list,
		struct evcpe_event **event,
		const char *event_code, const char *command_key);

inline void evcpe_event_list_remove(struct evcpe_event_list *list,
		struct evcpe_event *event);

void evcpe_event_list_remove_event(struct evcpe_event_list *list,
		const char *event_code);

struct evcpe_param_info {
	char name[257];
	int writable;
	TAILQ_ENTRY(evcpe_param_info) entry;
};

TAILQ_HEAD(evcpe_param_info_head, evcpe_param_info);

struct evcpe_param_info_list {
	struct evcpe_param_info_head head;
	unsigned int size;
};

inline void evcpe_param_info_list_init(struct evcpe_param_info_list *list);

void evcpe_param_info_list_clear(struct evcpe_param_info_list *list);

inline unsigned int evcpe_param_info_list_size(struct evcpe_param_info_list *list);

int evcpe_param_info_list_add(struct evcpe_param_info_list *list,
		struct evcpe_param_info **param, const char *name, unsigned len,
		int writable);

inline void evcpe_param_info_list_remove(struct evcpe_param_info_list *list,
		struct evcpe_param_info *param);

struct evcpe_param_name {
	char name[257];
	TAILQ_ENTRY(evcpe_param_name) entry;
};

TAILQ_HEAD(evcpe_param_name_head, evcpe_param_name);

struct evcpe_param_name_list {
	struct evcpe_param_name_head head;
	unsigned int size;
};

inline void evcpe_param_name_list_init(struct evcpe_param_name_list *list);

void evcpe_param_name_list_clear(struct evcpe_param_name_list *list);

inline unsigned int evcpe_param_name_list_size(struct evcpe_param_name_list *list);

int evcpe_param_name_list_add(struct evcpe_param_name_list *list,
		struct evcpe_param_name **param, const char *name, unsigned len);

inline void evcpe_param_name_list_remove(struct evcpe_param_name_list *list,
		struct evcpe_param_name *param);

struct evcpe_param_value {
	char name[257];
	enum evcpe_type type;
	const char *data;
	unsigned len;
	TAILQ_ENTRY(evcpe_param_value) entry;
};

int evcpe_param_value_set(struct evcpe_param_value *value,
		const char *data, unsigned len);

TAILQ_HEAD(evcpe_param_value_head, evcpe_param_value);

struct evcpe_param_value_list {
	struct evcpe_param_value_head head;
	unsigned int size;
};

inline void evcpe_param_value_list_init(struct evcpe_param_value_list *list);

void evcpe_param_value_list_clear(struct evcpe_param_value_list *list);

inline unsigned int evcpe_param_value_list_size(
		struct evcpe_param_value_list *list);

int evcpe_param_value_list_add(struct evcpe_param_value_list *list,
		struct evcpe_param_value **value, const char *name, unsigned len);

inline void evcpe_param_value_list_remove(struct evcpe_param_value_list *list,
		struct evcpe_param_value *value);

int evcpe_param_value_list_add_value(struct evcpe_param_value_list *list,
		const char *name, enum evcpe_type type, const char *value);

struct evcpe_access_list_item {
	char entity[65];
	TAILQ_ENTRY(evcpe_access_list_item) entry;
};

TAILQ_HEAD(evcpe_access_head, evcpe_access_list_item);

struct evcpe_access_list {
	struct evcpe_access_head head;
	unsigned int size;
};

void evcpe_access_list_init(struct evcpe_access_list *list);

void evcpe_access_list_clear(struct evcpe_access_list *list);

int evcpe_access_list_clone(struct evcpe_access_list *src,
		struct evcpe_access_list *dst);

inline unsigned int evcpe_access_list_size(struct evcpe_access_list *list);

int evcpe_access_list_add(struct evcpe_access_list *list,
		struct evcpe_access_list_item **item, const char *entity, unsigned len);

inline void evcpe_access_list_remove(struct evcpe_access_list *list,
		struct evcpe_access_list_item *item);

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

TAILQ_HEAD(evcpe_param_attr_head, evcpe_param_attr);

struct evcpe_param_attr_list {
	struct evcpe_param_attr_head head;
	unsigned int size;
};

void evcpe_param_attr_list_init(struct evcpe_param_attr_list *list);

void evcpe_param_attr_list_clear(struct evcpe_param_attr_list *list);

inline unsigned int evcpe_param_attr_list_size(
		struct evcpe_param_attr_list *list);

int evcpe_param_attr_list_add(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr **attr, const char *name, unsigned len);

void evcpe_param_attr_list_remove(struct evcpe_param_attr_list *list,
		struct evcpe_param_attr *attr);

struct evcpe_set_param_attr {
	char name[257];
	int notification_change;
	enum evcpe_notification notification;
	int access_list_change;
	struct evcpe_access_list access_list;
	TAILQ_ENTRY(evcpe_set_param_attr) entry;
};

TAILQ_HEAD(evcpe_set_param_attr_head, evcpe_set_param_attr);

struct evcpe_set_param_attr_list {
	struct evcpe_set_param_attr_head head;
	unsigned int size;
};

void evcpe_set_param_attr_list_init(struct evcpe_set_param_attr_list *list);

void evcpe_set_param_attr_list_clear(struct evcpe_set_param_attr_list *list);

inline unsigned int evcpe_set_param_attr_list_size(
		struct evcpe_set_param_attr_list *list);

int evcpe_set_param_attr_list_add(struct evcpe_set_param_attr_list *list,
		struct evcpe_set_param_attr **attr, const char *name, unsigned len);

inline void evcpe_set_param_attr_list_remove(struct evcpe_set_param_attr_list *list,
		struct evcpe_set_param_attr *attr);

struct evcpe_set_param_value {
	char name[257];
	const char *data;
	unsigned int len;
	TAILQ_ENTRY(evcpe_set_param_value) entry;
};

int evcpe_set_param_value_set(struct evcpe_set_param_value *param,
		const char *data, unsigned len);

TAILQ_HEAD(evcpe_set_param_value_head, evcpe_set_param_value);

struct evcpe_set_param_value_list {
	struct evcpe_set_param_value_head head;
	unsigned int size;
};

void evcpe_set_param_value_list_init(struct evcpe_set_param_value_list *list);

void evcpe_set_param_value_list_clear(struct evcpe_set_param_value_list *list);

int evcpe_set_param_value_list_add(struct evcpe_set_param_value_list *list,
		struct evcpe_set_param_value **value, const char *name, unsigned len);

inline unsigned int evcpe_set_param_value_list_size(
		struct evcpe_set_param_value_list *list);

struct evcpe_method_list_item {
	char name[65];
	TAILQ_ENTRY(evcpe_method_list_item) entry;
};

TAILQ_HEAD(evcpe_method_head, evcpe_method_list_item);

struct evcpe_method_list {
	struct evcpe_method_head head;
	unsigned int size;
};

void evcpe_method_list_init(struct evcpe_method_list *list);

void evcpe_method_list_clear(struct evcpe_method_list *list);

inline unsigned int evcpe_method_list_size(struct evcpe_method_list *list);

int evcpe_method_list_add(struct evcpe_method_list *list,
		struct evcpe_method_list_item **item, const char *name, unsigned len);

inline void evcpe_method_list_remove(struct evcpe_method_list *list,
		struct evcpe_method_list_item *item);

inline int evcpe_method_list_add_method(struct evcpe_method_list *list,
		const char *method);

#endif /* EVCPE_DATA_H_ */
