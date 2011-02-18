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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"

#include "xml.h"

int evcpe_xml_add_indent(struct evbuffer *buffer,
		const char *indent, unsigned int count)
{
	int rc;
	unsigned int i;

	for (i = 0; i < count; i ++) {
		if ((rc = evcpe_add_buffer(buffer, "%s", indent))) {
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_xml_add_string(struct evbuffer *buffer,
		const char *node, const char *string)
{
	return evcpe_add_buffer(buffer, "<%s>%s</%s>\n", node, string, node);
}

int evcpe_xml_add_int(struct evbuffer *buffer,
		const char *node, int value)
{
	return evcpe_add_buffer(buffer, "<%s>%d</%s>\n", node, value, node);
}

//int evcpe_xml_add_datetime(struct evbuffer *buffer,
//		const char *node, struct tm *datetime)
//{
//	int len;
//	char buf[32];
//
//	evcpe_debug(__func__, "adding date time node <%s>", node);
//	// TODO: timezone
//	len = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", datetime);
//	return evcpe_xml_add_string(buffer, node, buf);
//}

int evcpe_xml_add_xsd_string(struct evbuffer *buffer,
		const char *node, const char *string)
{
	return evcpe_add_buffer(buffer, "<%s xsi:type=\"xsd:string\">%s</%s>\n",
			node, string, node);
}

int evcpe_xml_add_xsd_boolean(struct evbuffer *buffer,
		const char *node, int value)
{
	return evcpe_add_buffer(buffer,
			"<%s xsi:type=\"xsd:boolean\">%s</%s>\n",
			node, value ? "true" : "false", node);
}

int evcpe_xml_add_xsd_int(struct evbuffer *buffer,
		const char *node, int value)
{
	return evcpe_add_buffer(buffer,
			"<%s xsi:type=\"xsd:int\">%d</%s>\n", node, value, node);
}

int evcpe_xml_add_xsd_unsigned_int(struct evbuffer *buffer,
		const char *node, unsigned int value)
{
	return evcpe_add_buffer(buffer,
			"<%s xsi:type=\"xsd:unsignedInt\">%d</%s>\n", node, value, node);
}
