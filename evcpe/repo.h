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

#include "obj.h"
#include "persister.h"
#include "inform.h"

struct evcpe_repo;

typedef void (*evcpe_repo_listen_cb)(struct evcpe_repo *repo,
		enum evcpe_attr_event event, const char *param_name, void *cbarg);

struct evcpe_repo_listener {
	evcpe_repo_listen_cb cb;
	void *cbarg;
	TAILQ_ENTRY(evcpe_repo_listener) entry;
};

TAILQ_HEAD(evcpe_repo_listeners, evcpe_repo_listener);

struct evcpe_repo {
	struct evcpe_obj *root;
	struct evcpe_repo_listeners listeners;
};

struct evcpe_repo *evcpe_repo_new(struct evcpe_obj *root);

void evcpe_repo_free(struct evcpe_repo *repo);

int evcpe_repo_listen(struct evcpe_repo *repo,
		evcpe_repo_listen_cb cb, void *arg);

int evcpe_repo_unlisten(struct evcpe_repo *repo,
		evcpe_repo_listen_cb cb);

int evcpe_repo_get_obj(struct evcpe_repo *repo, const char *name,
		struct evcpe_obj **obj);

int evcpe_repo_get(struct evcpe_repo *repo, const char *name,
		const char **value, unsigned int *len);

int evcpe_repo_set(struct evcpe_repo *repo, const char *name,
		const char *value, unsigned int len);

int evcpe_repo_getcpy(struct evcpe_repo *repo, const char *name,
		char *value, unsigned len);

const char *evcpe_repo_find(struct evcpe_repo *repo, const char *name);

int evcpe_repo_add_obj(struct evcpe_repo *repo, const char *name,
		unsigned int *index);

int evcpe_repo_del_obj(struct evcpe_repo *repo, const char *name);

int evcpe_repo_get_objs(struct evcpe_repo *repo, const char *name,
		struct evcpe_obj_list **list, unsigned int *size);

int evcpe_repo_add_event(struct evcpe_repo *repo,
		const char *event_code, const char *command_key);

int evcpe_repo_del_event(struct evcpe_repo *repo,
		const char *event_code);

int evcpe_repo_to_inform(struct evcpe_repo *repo, struct evcpe_inform *inform);

int evcpe_repo_to_param_info_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_info_list *list, int next_level);

int evcpe_repo_to_param_attr_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_attr_list *list);

int evcpe_repo_to_param_value_list(struct evcpe_repo *repo, const char *name,
		struct evcpe_param_value_list *list);

#endif /* EVCPE_REPO_H_ */
