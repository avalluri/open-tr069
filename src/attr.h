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
#include "tqueue.h"

typedef struct _evcpe_attr evcpe_attr;
typedef struct _evcpe_obj evcpe_obj;

enum evcpe_attr_event {
	EVCPE_ATTR_EVENT_PARAM_SET,
	EVCPE_ATTR_EVENT_OBJ_ADDED,
	EVCPE_ATTR_EVENT_OBJ_DELETED
};

typedef void (*evcpe_attr_cb_t)(evcpe_attr *attr,
		enum evcpe_attr_event event, int inform_acs,
		void *data, void *cbarg);

struct _evcpe_attr {
	char *path;
	unsigned int pathlen;
	evcpe_obj *owner;
	evcpe_attr_schema *schema;
	evcpe_attr_cb_t cb;
	void *cbarg;
	union {
		struct {
			char *string;
			evcpe_notification_t notification;
			tqueue* access_list;
		} simple;
		evcpe_obj *object;
		struct {
			tqueue* list;
			unsigned size;
			unsigned max;
		} multiple;
	} value;
	RB_ENTRY(_evcpe_attr) entry;
} ;

RB_HEAD(_evcpe_attrs, _evcpe_attr);

typedef struct _evcpe_attrs evcpe_attrs;

evcpe_attr *evcpe_attr_new(evcpe_obj *owner, evcpe_attr_schema *schema);

void evcpe_attr_free(evcpe_attr *attr);

int evcpe_attr_cmp(evcpe_attr *a, evcpe_attr *b);

RB_PROTOTYPE(_evcpe_attrs, _evcpe_attr, next, evcpe_attr_cmp);

int evcpe_attr_init(evcpe_attr *attr);

void evcpe_attr_set_cb(evcpe_attr *attr, evcpe_attr_cb_t cb, void *arg);

int evcpe_attr_set_notification(evcpe_attr *attr,
		evcpe_notification_t notification);

int evcpe_attr_set_access_list(evcpe_attr *attr, tqueue* list);

int evcpe_attr_set_access_list_from_str(evcpe_attr *attr,
		const char *value, unsigned len);

void evcpe_attr_unset(evcpe_attr *attr);

int evcpe_attr_set(evcpe_attr *attr, const char *value, unsigned len);

int evcpe_attr_get(evcpe_attr *attr, const char **value, unsigned int *len);

//int evcpe_attr_set_obj(evcpe_attr *attr, evcpe_obj **child);
//
int evcpe_attr_get_obj(evcpe_attr *attr, evcpe_obj **child);

int evcpe_attr_add_obj(evcpe_attr *attr,
		evcpe_obj **child, unsigned int *index);

int evcpe_attr_idx_obj(evcpe_attr *attr,
		unsigned int index, evcpe_obj **child);

int evcpe_attr_del_obj(evcpe_attr *attr, unsigned int index);

int evcpe_attr_to_param_value_list(evcpe_attr *attr, tqueue *list);

int evcpe_attr_to_param_info_list(evcpe_attr *attr, tqueue *list,
		int next_level);

int evcpe_attr_to_param_attr_list(evcpe_attr *attr, tqueue *list);

/* attribute list */
void evcpe_attrs_init(evcpe_attrs *attrs);

void evcpe_attrs_clear(evcpe_attrs *attrs);


#endif /* EVCPE_ATTR_H_ */
