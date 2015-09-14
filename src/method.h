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

#ifndef EVCPE_METHOD_H_
#define EVCPE_METHOD_H_

#include <time.h>

#include "data.h"

enum evcpe_method_type {
	EVCPE_UNKNOWN_METHOD,
	EVCPE_GET_RPC_METHODS,
	EVCPE_SET_PARAMETER_VALUES,
	EVCPE_GET_PARAMETER_VALUES,
	EVCPE_GET_PARAMETER_NAMES,
	EVCPE_SET_PARAMETER_ATTRIBUTES,
	EVCPE_GET_PARAMETER_ATTRIBUTES,
	EVCPE_ADD_OBJECT,
	EVCPE_DELETE_OBJECT,
	EVCPE_REBOOT,
	EVCPE_DOWNLOAD,
	EVCPE_UPLOAD,
	EVCPE_FACTORY_RESET,
	EVCPE_GET_QUEUED_TRANSFERS,
	EVCPE_GET_ALL_QUEUED_TRANSFERS,
	EVCPE_SCHEDULE_INFORM,
	EVCPE_SET_VOUCHERS,
	EVCPE_GET_OPTIONS,
	EVCPE_INFORM,
	EVCPE_TRANSFER_COMPLETE,
	EVCPE_AUTONOMOUS_TRANSFER_COMPLETE,
	EVCPE_REQUEST_DOWNLOAD,
	EVCPE_KICKED
};

const char *evcpe_method_type_to_str(enum evcpe_method_type type);

#endif /* EVCPE_METHOD_H_ */
