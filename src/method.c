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

#include "method.h"

const char *evcpe_method_type_to_str(enum evcpe_method_type type)
{
	switch(type) {
	case EVCPE_GET_RPC_METHODS:
		return "GetRPCMethods";
	case EVCPE_SET_PARAMETER_VALUES:
		return "SetParameterValues";
	case EVCPE_GET_PARAMETER_VALUES:
		return "GetParameterValues";
	case EVCPE_GET_PARAMETER_NAMES:
		return "GetParameterNames";
	case EVCPE_SET_PARAMETER_ATTRIBUTES:
		return "SetParameterAttributes";
	case EVCPE_GET_PARAMETER_ATTRIBUTES:
		return "GetParameterAttributes";
	case EVCPE_ADD_OBJECT:
		return "AddObject";
	case EVCPE_DELETE_OBJECT:
		return "DeleteObject";
	case EVCPE_REBOOT:
		return "Reboot";
	case EVCPE_DOWNLOAD:
		return "Download";
	case EVCPE_UPLOAD:
		return "Upload";
	case EVCPE_FACTORY_RESET:
		return "FactoryReset";
	case EVCPE_GET_QUEUED_TRANSFERS:
		return "GetQueuedTransfers";
	case EVCPE_GET_ALL_QUEUED_TRANSFERS:
		return "GetAllQueuedTransfers";
	case EVCPE_SCHEDULE_INFORM:
		return "ScheduleInform";
	case EVCPE_SET_VOUCHERS:
		return "SetVouchers";
	case EVCPE_GET_OPTIONS:
		return "GetOptions";
	case EVCPE_INFORM:
		return "Inform";
	case EVCPE_TRANSFER_COMPLETE:
		return "TransferComplete";
	case EVCPE_AUTONOMOUS_TRANSFER_COMPLETE:
		return "AutonomousTransferComplete";
	case EVCPE_REQUEST_DOWNLOAD:
		return "RequestDownload";
	case EVCPE_KICKED:
		return "Kicked";
	default:
		return "unknown";
	}
}
