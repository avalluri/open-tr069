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

#ifndef EVCPE_TYPE_H_
#define EVCPE_TYPE_H_

#include <sys/types.h>

#include "constraint.h"

enum evcpe_type {
	EVCPE_TYPE_UNKNOWN,
	EVCPE_TYPE_OBJECT,
	EVCPE_TYPE_MULTIPLE,
	EVCPE_TYPE_STRING,
	EVCPE_TYPE_INT,
	EVCPE_TYPE_UNSIGNED_INT,
	EVCPE_TYPE_BOOLEAN,
	EVCPE_TYPE_DATETIME,
	EVCPE_TYPE_BASE64
};

const char *evcpe_type_to_str(enum evcpe_type type);

int evcpe_type_validate(enum evcpe_type type, const char *value, unsigned len,
		struct evcpe_constraint *cons);

#endif /* EVCPE_TYPE_H_ */
