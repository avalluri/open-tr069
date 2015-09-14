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

#ifndef EVCPE_ERROR_H_
#define EVCPE_ERROR_H_

enum evcpe_error_type {
	EVCPE_ERR_SYSTEM,
	EVCPE_ERR_DNS,
	EVCPE_ERR_HTTP,
	EVCPE_ERR_SOAP,
	EVCPE_ERR_CWMP
};

enum evcpe_session_error {
	EVCPE_DNS_ERR_FORMAT,
	EVCPE_DNS_ERR_SERVER_FAILED,
	EVCPE_DNS_ERR_NOT_EXIST,
	EVCPE_DNS_ERR_NOT_IMPL,
	EVCPE_DNS_ERR_REFUSED,
	EVCPE_DNS_ERR_TIMED_OUT,
	EVCPE_DNS_ERR_INVALID,
	EVCPE_DNS_ERR_NO_ADDRESS,
	EVCPE_DNS_ERR_OTHER
};

#endif /* EVCPE_ERROR_H_ */
