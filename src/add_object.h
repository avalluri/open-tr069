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

#ifndef EVCPE_ADD_OBJECT_H_
#define EVCPE_ADD_OBJECT_H_

struct evcpe_add_object {
	char object_name[257];
	char parameter_key[33];
};

struct evcpe_add_object *evcpe_add_object_new(void);

void evcpe_add_object_free(struct evcpe_add_object *method);

struct evcpe_add_object_response {
	unsigned int instance_number;
	int status;
};

struct evcpe_add_object_response *evcpe_add_object_response_new(void);

void evcpe_add_object_response_free(struct evcpe_add_object_response *method);

#endif /* EVCPE_ADD_OBJECT_H_ */
