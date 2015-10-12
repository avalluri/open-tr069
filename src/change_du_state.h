/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of open-tr069
 *
 * Copyright (C) 2013-2015 Intel Corporation.
 *
 * Contact: Amarnath Valluri <amarnath.valluri@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef EVCPE_CHANGE_DU_STATE_H_
#define EVCPE_CHANGE_DU_STATE_H_

typedef enum {
	EVCPE_CHANGE_DU_STATE_INSTALL,
	EVCPE_CHANGE_DU_STATE_UPDATE,
	EVCPE_CHANGE_DU_STATE_UNINSTALL
} evcpe_chagne_du_state_type_t;

typedef struct _install {
	evcpe_url url;
	char uuid[36];
	char username[256];
	char password[256];
	char ee_ref[256];
} evcpe_change_du_state_install;

typedef struct _update {
	char uuid[36];
	char version[32];
	evcpe_url url;
	char username[256];
	char password[256];
} evcpe_change_du_state_update;

typedef struct _uninstall {
	char uuid[36];
	char version[32];
	char ee_ref[256];
} evcpe_change_du_state_uninstall;

typedef struct {
	evcpe_chagne_du_state_type_t type;
	union {
		evcpe_change_du_state_install   install;
		evcpe_change_du_state_update    update;
		evcpe_change_du_state_uninstall uninstall;
	} operation;

} evcpe_change_du_state_operation;


typedef struct _change_du_state
{
	char command_key[32];
	tqueue *operations; // list of operations
} evcpe_change_du_state;

evcpe_change_du_state* evcpe_change_du_state_new();

void evcpe_change_du_state_free(evcpe_change_du_state* req);

#endif /* EVCPE_CHANGE_DU_STATE_H_ */
