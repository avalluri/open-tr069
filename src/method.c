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

const char *methods[] = {
		[EVCPE_GET_RPC_METHODS] = "GetRPCMethods",
		[EVCPE_SET_PARAMETER_VALUES] = "SetParameterValues",
		[EVCPE_GET_PARAMETER_VALUES] = "GetParameterValues",
		[EVCPE_GET_PARAMETER_NAMES] = "GetParameterNames",
		[EVCPE_SET_PARAMETER_ATTRIBUTES] = "SetParameterAttributes",
		[EVCPE_GET_PARAMETER_ATTRIBUTES] = "GetParameterAttributes",
		[EVCPE_ADD_OBJECT] = "AddObject",
		[EVCPE_DELETE_OBJECT] = "DeleteObject",
		[EVCPE_REBOOT] = "Reboot",
		[EVCPE_DOWNLOAD] = "Download",
		[EVCPE_UPLOAD] = "Upload",
		[EVCPE_FACTORY_RESET] = "FactoryReset",
		[EVCPE_GET_QUEUED_TRANSFERS] = "GetQueuedTransfers",
		[EVCPE_GET_ALL_QUEUED_TRANSFERS] = "GetAllQueuedTransfers",
		[EVCPE_SCHEDULE_INFORM] = "ScheduleInform",
		[EVCPE_SET_VOUCHERS] = "SetVouchers",
		[EVCPE_GET_OPTIONS] = "GetOptions",
		[EVCPE_INFORM] = "Inform",
		[EVCPE_TRANSFER_COMPLETE] = "TransferComplete",
		[EVCPE_AUTONOMOUS_TRANSFER_COMPLETE] = "AutonomousTransferComplete",
		[EVCPE_REQUEST_DOWNLOAD] = "RequestDownload",
		[EVCPE_KICKED] = "Kicked",
		[EVCPE_UNKNOWN_METHOD] = "Unknown"
};

const char* evcpe_method_type_to_str(evcpe_method_type_t type)
{
	if (type < 0 || type >= EVCPE_UNKNOWN_METHOD) return "Unknown";

	return methods[type];
}


evcpe_method_type_t evcpe_method_type_from_str(const char *method,
		unsigned len)
{
  int i = 0;

  if (!method || !len) return EVCPE_UNKNOWN_METHOD;

  for(i = 0; i < EVCPE_UNKNOWN_METHOD; i++) {
		if (!evcpe_strncmp(methods[i], method, len)) {
      return (evcpe_method_type_t)i;
    }
  }

  return EVCPE_TYPE_UNKNOWN;
}
