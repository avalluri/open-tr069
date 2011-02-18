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

#include <time.h>
#include <errno.h>

#include "log.h"

#include "accessor.h"

int evcpe_get_curtime(struct evcpe_attr *attr,
		const char **value, unsigned int *len)
{
	char buf[32];
	time_t curtime;

	evcpe_debug(__func__, "get current time");
	// TODO: timezone
	if ((curtime = time(NULL)) == ((time_t)-1)) {
		evcpe_error(__func__, "failed to get time");
		return errno;
	}
 	*len = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&curtime));
	*value = buf;
	return 0;
}

int evcpe_set_curtime(struct evcpe_attr *attr,
		const char *buffer, unsigned int len)
{
	evcpe_error(__func__, "not implemented");
	return -1;
}
