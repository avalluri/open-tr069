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

#ifndef EVCPE_FAULT_H_
#define EVCPE_FAULT_H_

#include <event.h>

typedef enum _evcpe_fault_code {
	EVCEP_FAULT_CODE_NONE = 0,
	EVCPE_ACS_FAULT_MIN = 8000,
	EVCPE_ACS_METHOD_NOT_SUPPORTED = 8000,
	EVCPE_ACS_REQUEST_DENIED = 8001,
	EVCPE_ACS_INTERNAL_ERROR = 8002,
	EVCPE_ACS_INVALID_ARGUMENTS = 8003,
	EVCPE_ACS_RESOUCES_EXCEEDS = 8004,
	EVCPE_ACS_RETRY_REQUEST = 8005,
	EVCPE_ACS_VERSION_INCOMPATIBLE = 8006,
	EVCPE_ACS_FAULT_MAX = 8899,
	EVCPE_CPE_FAULT_MIN = 9000,
	EVCPE_CPE_METHOD_NOT_SUPPORTED = 9000,
	EVCPE_CPE_REQUEST_DENIED = 9001,
	EVCPE_CPE_INTERNAL_ERROR = 9002,
	EVCPE_CPE_INVALID_ARGUMENTS = 9003,
	EVCPE_CPE_RESOUCES_EXCEEDS = 9004,
	EVCPE_CPE_INVALID_PARAM_NAME = 9005,
	EVCPE_CPE_INVALID_PARAM_TYPE = 9006,
	EVCPE_CPE_INVALID_PARAM_VALUE = 9007,
	EVCPE_CPE_NON_WRITABLE_PARAM = 9008,
	EVCPE_CPE_NOTIFICATION_REQUEST_REJECTED = 9009,
	EVCPE_CPE_DOWNLOAD_FAILURE = 9010,
	EVCPE_CPE_UPLOAD_FAILURE = 9011,
	EVCPE_CPE_FILE_SERVER_AUTH_FAILURE = 9012,
	EVCPE_CPE_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,
	EVCPE_CPE_DOWNLOAD_FAILURE_UNABLE_TO_JOIN_MULTICAST = 9014,
	EVCPE_CPE_DOWNLOAD_FAILURE_UNABLE_TO_CONTACT_SERVER = 9015,
	EVCPE_CPE_DOWNLOAD_FAILURE_UNABLE_TO_ACCESS_FILE = 9016,
	EVCPE_CPE_DOWNLOAD_FAILURE_UNABLE_TO_COMPLETE = 9017,
	EVCPE_CPE_DOWNLOAD_FAILURE_FILE_CORRUPTED = 9018,
	EVCPE_CPE_DOWNLOAD_FAILURE_FILE_AUTH_FAILURE = 9019,
	EVCPE_CPE_DOWNLOAD_FAILURE_TIMEOUT = 9020,
	EVCPE_CPE_FAILURE_CANCEL_FILE_TRANSFER = 9021,
	EVCPE_CPE_INVALID_UUID = 9022,
	EVCPE_CPE_UNKNOWN_EE = 9023,
	EVCPE_CPE_DISABLED_EE = 9024,
	EVCPE_CPE_DU_TO_EE_MISMATCH = 9025,
	EVCPE_CPE_DUPLICATE_DU = 9026,
	EVCPE_CPE_SYSTEM_RESOURCES_EXCEEDED = 9027,
	EVCPE_CPE_UNKNOWN_DU = 9028,
	EVCPE_CPE_INVALID_DU_STATE_= 9029,
	EVCPE_CPE_INVALID_DU_UPDATE_DOWNGRATE_NOT_PERMITTED = 9030,
	EVCPE_CPE_INVALID_DU_UPDATE_NO_VERSION = 9031,
	EVCPE_CPE_INVALID_DU_UPDATE_VERSION_EXISTS = 9032,
	EVCPE_CPE_FAULT_MAX = 9899
} evcpe_fault_code_t;

typedef struct _evcpe_fault {
	evcpe_fault_code_t code;
	char string[256];
	void *detail;
} evcpe_fault;

evcpe_fault *evcpe_fault_new(void);

void evcpe_fault_free(evcpe_fault *fault);

int evcpe_fault_to_xml(evcpe_fault *fault, struct evbuffer *buffer);

#endif /* EVCPE_FAULT_H_ */
