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
#include <string.h>

#include "type.h"
#include "tqueue.h"

typedef struct _evcpe_device_id {
	char manufacturer[65];
	char oui[7];
	char product_class[65];
	char serial_number[65];
} evcpe_device_id;

typedef enum _evcpe_event_code {
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
  EVCPE_EVENT_M_REBOOT,
  EVCPE_EVENT_M_SCHEDULE_INFORM,
  EVCPE_EVENT_M_DOWNLOAD,
  EVCPE_EVENT_M_SCHEDULE_DOWNLOAD,
  EVCPE_EVENT_M_UPLOAD,
  EVCPE_EVENT_M_CHANGE_DU_STATE,
  EVCPE_EVENT_M_X_VENDOR_EVENT,
  EVCPE_EVENT_MAX = 999
} evcpe_event_code_t;

typedef struct _evcpe_event {
	evcpe_event_code_t code;
	char command_key[33];
} evcpe_event;

evcpe_event* evcpe_event_new(evcpe_event_code_t c, const char* key);
const char* evcpe_event_code_to_str(evcpe_event_code_t code);
int evcpe_compare_event(evcpe_event *ev, evcpe_event_code_t code);

#define evcpe_event_list_new() tqueue_new((tqueue_compare_func_t)\
		evcpe_compare_event, free)

evcpe_event* evcpe_event_list_add(tqueue* list,
		evcpe_event_code_t code, const char *command_key);


typedef struct _evcpe_param_info {
	char name[257];
	int writable;
} evcpe_param_info;

evcpe_param_info* evcpe_param_info_new(const char*name, int len, int writable);
void evcpe_param_info_free(evcpe_param_info* info);
int evcpe_param_info_compare(evcpe_param_info* param, const char* name);

#define evcpe_param_info_list_new() \
	tqueue_new((tqueue_compare_func_t)evcpe_param_info_compare, \
			(tqueue_free_func_t)evcpe_param_info_free)

evcpe_param_info* evcpe_param_info_list_add(tqueue* list, const char* name,
		int len, int write);

typedef struct _evcpe_param_value {
	char name[257];
	evcpe_type_t type;
	const char *data;
	unsigned len;
} evcpe_param_value;

evcpe_param_value* evcpe_param_value_new(const char* name,
		unsigned name_len, const char* value, unsigned value_len,
		evcpe_type_t type);
void evcpe_param_value_free(evcpe_param_value* pv);

#define evcpe_param_value_list_new() \
	tqueue_new(NULL, (tqueue_free_func_t)evcpe_param_value_free)

typedef enum _evcpe_notification {
	EVCPE_NOTIFICATION_OFF,
	EVCPE_NOTIFICATION_PASSIVE,
	EVCPE_NOTIFICATION_ACTIVE,
	EVCPE_NOTIFICATION_PASSIVE_LIGHTWEIGHT,
	EVCPE_NOTIFICATION_PASSIVE_LIGHTWEIGHT_PASSIVE,
	EVCPE_NOTIFICATION_ACTIVE_LIGHTWEIGHT,
	EVCPE_NOTIFICATION_PASSIVE_LIGHTWEIGHT_ACTIVE,
	EVCPE_NOTIFICATION_FORCED_ACTIVE,
	EVCPE_NOTIFICATION_UNKNOWN
} evcpe_notification_t;

typedef struct _evcpe_param_attr {
	char name[257];
	evcpe_notification_t notification;
	tqueue* access_list;
} evcpe_param_attr;

evcpe_param_attr* evcpe_param_attr_new(const char* name, unsigned len,
		evcpe_notification_t notification);

void evcpe_param_attr_free(evcpe_param_attr* pa);

#define evcpe_param_attr_list_new() \
	tqueue_new(NULL, (tqueue_free_func_t)evcpe_param_attr_free)

typedef struct _evcpe_set_param_attr {
	int notification_change;
	int access_list_change;
	evcpe_param_attr *info;
} evcpe_set_param_attr;

evcpe_set_param_attr* evcpe_set_param_attr_new(const char* name, unsigned len);
void evcpe_set_param_attr_free(evcpe_set_param_attr* info);

#define evcpe_set_param_attr_list_new() \
	tqueue_new(NULL, (tqueue_free_func_t)evcpe_set_param_attr_free);

#endif /* EVCPE_DATA_H_ */
