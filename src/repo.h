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

#ifndef EVCPE_REPO_H_
#define EVCPE_REPO_H_

#include "data.h"
#include "obj.h"
#include "inform.h"
#include "tqueue.h"

typedef struct _evcpe_repo evcpe_repo;

typedef void (*evcpe_repo_listen_cb_t)(evcpe_repo *repo,
		enum evcpe_attr_event event, const char *param_name, void *cbarg);

typedef struct _evcpe_repo_listener {
	evcpe_repo_listen_cb_t cb;
	void *cbarg;
} evcpe_repo_listener;

struct _evcpe_repo {
	evcpe_obj* root;
	tqueue* forced_inform_attrs;
	tqueue* changed_atts;
	tqueue* pending_events;
	tqueue* listeners;
};

evcpe_repo *evcpe_repo_new(evcpe_obj *root);

void evcpe_repo_free(evcpe_repo *repo);

int evcpe_repo_init(evcpe_repo* repo);

int evcpe_repo_listen(evcpe_repo *repo,
		evcpe_repo_listen_cb_t cb, void *arg);

int evcpe_repo_unlisten(evcpe_repo *repo,
		evcpe_repo_listen_cb_t cb);

int evcpe_repo_get_obj(evcpe_repo *repo, const char *name,
		evcpe_obj **obj);

int evcpe_repo_get(evcpe_repo *repo, const char *name,
		const char **value, unsigned int *len);

int evcpe_repo_set(evcpe_repo *repo, const char *name,
		const char *value, unsigned int len);

int evcpe_repo_getcpy(evcpe_repo *repo, const char *name,
		char *value, unsigned len);

const char *evcpe_repo_find(evcpe_repo *repo, const char *name);

int evcpe_repo_add_obj(evcpe_repo *repo, const char *name,
		unsigned int *index);

int evcpe_repo_del_obj(evcpe_repo *repo, const char *name);

int evcpe_repo_get_objs(evcpe_repo *repo, const char *name,
		tqueue **list, unsigned int *size);

int evcpe_repo_add_event(evcpe_repo *repo,
		evcpe_event_code_t code, const char *command_key);

int evcpe_repo_del_event(evcpe_repo *repo, evcpe_event_code_t code);

int evcpe_repo_to_inform(evcpe_repo *repo, evcpe_inform *inform);

int evcpe_repo_to_param_info_list(evcpe_repo *repo, const char *name,
		tqueue *list, int next_level);

int evcpe_repo_to_param_attr_list(evcpe_repo *repo, const char *name,
		tqueue *list);

int evcpe_repo_to_param_value_list(evcpe_repo *repo, const char *name,
		tqueue *list);

void evcpe_repo_clear_pending_events(evcpe_repo* repo);

#endif /* EVCPE_REPO_H_ */
