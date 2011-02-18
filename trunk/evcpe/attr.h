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

#ifndef EVCPE_ATTR_H_
#define EVCPE_ATTR_H_

#include <sys/tree.h>

#include "data.h"
#include "class.h"
#include "obj_list.h"

struct evcpe_attr;

enum evcpe_attr_event {
	EVCPE_ATTR_EVENT_PARAM_SET,
	EVCPE_ATTR_EVENT_OBJ_ADDED,
	EVCPE_ATTR_EVENT_OBJ_DELETED
};

typedef void (*evcpe_attr_cb)(struct evcpe_attr *attr,
		enum evcpe_attr_event event, void *data, void *cbarg);

struct evcpe_attr {
	char *path;
	unsigned int pathlen;
	struct evcpe_obj *owner;
	struct evcpe_attr_schema *schema;
	evcpe_attr_cb cb;
	void *cbarg;
	union {
		struct {
			char *string;
			enum evcpe_notification notification;
			struct evcpe_access_list access_list;
		} simple;
		struct evcpe_obj *object;
		struct {
			struct evcpe_obj_list list;
			unsigned size;
			unsigned max;
		} multiple;
	} value;
	RB_ENTRY(evcpe_attr) entry;
};

RB_HEAD(evcpe_attrs, evcpe_attr);

int evcpe_attr_cmp(struct evcpe_attr *a, struct evcpe_attr *b);

RB_PROTOTYPE(evcpe_attrs, evcpe_attr, next, evcpe_attr_cmp);

int evcpe_attr_init(struct evcpe_attr *attr);

void evcpe_attr_set_cb(struct evcpe_attr *attr, evcpe_attr_cb cb, void *arg);

int evcpe_attr_set_notification(struct evcpe_attr *attr,
		enum evcpe_notification notification);

void evcpe_attr_unset(struct evcpe_attr *attr);

int evcpe_attr_set(struct evcpe_attr *attr, const char *value, unsigned len);

int evcpe_attr_get(struct evcpe_attr *attr, const char **value, unsigned int *len);

//int evcpe_attr_set_obj(struct evcpe_attr *attr, struct evcpe_obj **child);
//
int evcpe_attr_get_obj(struct evcpe_attr *attr, struct evcpe_obj **child);

int evcpe_attr_add_obj(struct evcpe_attr *attr,
		struct evcpe_obj **child, unsigned int *index);

int evcpe_attr_idx_obj(struct evcpe_attr *attr,
		unsigned int index, struct evcpe_obj **child);

int evcpe_attr_del_obj(struct evcpe_attr *attr, unsigned int index);

int evcpe_attr_to_param_value_list(struct evcpe_attr *attr,
		struct evcpe_param_value_list *list);

int evcpe_attr_to_param_info_list(struct evcpe_attr *attr,
		struct evcpe_param_info_list *list, int next_level);

int evcpe_attr_to_param_attr_list(struct evcpe_attr *attr,
		struct evcpe_param_attr_list *list);

#endif /* EVCPE_ATTR_H_ */
