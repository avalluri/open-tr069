// $Id$
/*
 * Copyright (C) 2015 Intel Corporation
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
#include "data.h"

const char *evcpe_event_code_to_str(evcpe_event_code_t code) {
  switch(code) {
    case EVCPE_EVENT_0_BOOTSTRAP: return "0 BOOTSTRAP";
    case EVCPE_EVENT_1_BOOT: return "1 BOOT";
    case EVCPE_EVENT_2_PERIODIC: return "2 PERIODIC";
    case EVCPE_EVENT_3_SCHEDULED: return "3 SCHEDULED";
    case EVCPE_EVENT_4_VALUE_CHANGE: return "4 VALUE CHANGE";
    case EVCPE_EVENT_5_KICKED: return "5 KICKED";
    case EVCPE_EVENT_6_CONNECTION_REQUEST: return "6 CONNECTION REQUEST";
    case EVCPE_EVENT_7_TRANSFER_COMPLETE: return "7 TRANSFER COMPLETE";
    case EVCPE_EVENT_8_DIAGNOSTICS_COMPLETE: return "8 DIAGNOSTICS COMPLETE";
    case EVCPE_EVENT_9_REQUEST_DOWNLOAD: return "9 REQUEST DOWNLOAD";
    case EVCPE_EVENT_10_AUTONOMOUS_TRANSFER_COMPLETE: return "10 AUTONOMOUS TRANSFER COMPLETE";
    case EVCPE_EVENT_11_DU_STATE_CHANGE_COMPLETE: return "11 DU STATE CHANGE COMPLETE";
    case EVCPE_EVENT_12_AUTONOMOUS_DU_STATE_CHANGE_COMPLETE: return "12 AUTONOMOUS DU STATE CHANGE COMPLETE";
    case EVCPE_EVENT_13_WAKEUP: return "13 WAKEUP";
    case EVCPE_EVENT_M_REBOOT: return "M Reboot";
    default: return NULL;
  }
}

evcpe_event *evcpe_event_new(evcpe_event_code_t code,
		const char* command_key)
{
	evcpe_event* ev = calloc(1, sizeof(evcpe_event));
	if (!ev) return NULL;

	ev->code = code;
	if (command_key) {
		size_t len = sizeof(ev->command_key) - 1;
		strncmp(ev->command_key, command_key, len);
		ev->command_key[len] = '\0';
	}

	return ev;
}

void evcpe_event_free(evcpe_event* ev) {
	if (ev) free(ev);
}

int evcpe_compare_event(evcpe_event *ev, evcpe_event_code_t code) {
	return ev->code == code ? 0 : 1;
}

evcpe_event * evcpe_event_list_add(tqueue *list,
		evcpe_event_code_t code, const char *command_key)
{
	evcpe_event *ev = NULL;

	if (!list) return NULL;
	if (command_key && (strlen(command_key) >= sizeof(ev->command_key)))
		return NULL;

	if ((code <= EVCPE_EVENT_13_WAKEUP) &&
		 (tqueue_find(list, (void*)code) != NULL)) {
		return NULL;
	}

	TRACE("adding event: %s", evcpe_event_code_to_str(code));

	if (!(ev = evcpe_event_new(code, command_key))) {
		ERROR("failed to calloc evcpe_event");
		return NULL;
	}

	tqueue_insert(list, ev);

	return ev;
}

evcpe_param_info* evcpe_param_info_new(const char* name, int len, int writable)
{
	evcpe_param_info* info = NULL;

	if (!name || !len) return NULL;
	if (len == -1) len = strlen(name);
	if (len >= sizeof(info->name)) return NULL;

	info = (evcpe_param_info*)calloc(1, sizeof(*info));
	if (!info) return NULL;

	strncpy(info->name, name, len);
	info->writable = writable;

	return info;
}

void evcpe_param_info_free(evcpe_param_info* info) {
	if (info) free(info);
}

int evcpe_param_info_compare(evcpe_param_info* param, const char* name) {
	if (param && name) return strncmp(param->name, name, sizeof(param->name));
	return 1;
}

evcpe_param_info* evcpe_param_info_list_add(tqueue* list, const char* name,
		int len, int write)
{
	int rc = 0;
	evcpe_param_info* i = NULL;

	if (!list) return NULL;

	if (!(i = evcpe_param_info_new(name, len, write))) return NULL;

	if (!tqueue_insert(list, i)) {
		evcpe_param_info_free(i);
		return NULL;
	}

	return i;
}

evcpe_param_value* evcpe_param_value_new(const char* name,
		unsigned name_len, const char* value, unsigned value_len,
		evcpe_type_t type) {
	evcpe_param_value* pv = NULL;

	if (!name || !name_len) return NULL;
	if (name_len > sizeof(pv->name)) return NULL;

	pv = (evcpe_param_value*)calloc(1, sizeof(*pv));
	if (!pv) return NULL;
	strncpy(pv->name, name, name_len);
	pv->data = value;
	pv->len = value_len;
	pv->type = type;

	return pv;
}

void evcpe_param_value_free(evcpe_param_value* pv) {
	if (pv) free(pv);
}

evcpe_param_attr* evcpe_param_attr_new(const char* name, unsigned len,
		evcpe_notification_t notification) {
	evcpe_param_attr* pa = NULL;
	if (!name || !len) return NULL;
	if (len >= sizeof(pa->name)) return NULL;

	pa = (evcpe_param_attr*) calloc(1, sizeof(*pa));
	if (!pa) return NULL;

	strncpy(pa->name, name, len);
	pa->notification = notification;
	pa->access_list = tqueue_new(NULL, free);

	return pa;
}

void evcpe_param_attr_free(evcpe_param_attr* pa) {
	if (pa) {
		tqueue_free(pa->access_list);
		free(pa);
	}
}

evcpe_set_param_attr* evcpe_set_param_attr_new(const char* name,
		unsigned len) {
	evcpe_set_param_attr* info = calloc(1, sizeof(*info));
	if(!info) return NULL;

	info->info = evcpe_param_attr_new(name, len, EVCPE_NOTIFICATION_UNKNOWN);
	if (!info->info) {
		free(info); return NULL;
	}
	return info;
}

void evcpe_set_param_attr_free(evcpe_set_param_attr* info) {
	if (!info) return;
	if (info->info) evcpe_param_attr_free(info->info);
	free(info);
}
