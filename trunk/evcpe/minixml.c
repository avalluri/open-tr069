/* $Id$ */
/* minixml.c : the minimum size a xml parser can be ! */
/* Project : miniupnp
 * webpage: http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * Author : Thomas Bernard

Copyright (c) 2005-2007, Thomas BERNARD
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#include "minixml.h"

#define MINIXML_ENSURE_MORE do \
	if(p->xml >= p->xmlend) { \
		evcpe_error(__func__, "unexpected end of data"); \
		return EPROTO; \
	} \
	while(0)

/* parseatt : used to parse the argument list
 * return 0 (false) in case of success and -1 (true) if the end
 * of the xmlbuffer is reached. */
int parseatt(struct xmlparser * p)
{
	int rc;
	const char * ns;
	unsigned nslen;
	const char * attname;
	unsigned attnamelen;
	const char * attvalue;
	unsigned attvaluelen;
	while(p->xml < p->xmlend)
	{
		if(*p->xml=='/' || *p->xml=='>')
			return 0;
		if( !IS_WHITE_SPACE(*p->xml) )
		{
			char sep;
			ns = NULL;
			nslen = 0;
			attname = p->xml;
			attnamelen = 0;
			while(*p->xml!='=' && !IS_WHITE_SPACE(*p->xml) )
			{
				if(*p->xml==':')
				{
					ns = attname;
					nslen = attnamelen;
					attnamelen = 0;
					attname = ++p->xml;
				} else {
					attnamelen++; p->xml++;
				}
				MINIXML_ENSURE_MORE;
			}
			while(*(p->xml++) != '=')
			{
				MINIXML_ENSURE_MORE;
			}
			while(IS_WHITE_SPACE(*p->xml))
			{
				p->xml++;
				MINIXML_ENSURE_MORE;
			}
			sep = *p->xml;
			if(sep=='\'' || sep=='\"')
			{
				p->xml++;
				MINIXML_ENSURE_MORE;
				attvalue = p->xml;
				attvaluelen = 0;
				while(*p->xml != sep)
				{
					attvaluelen++; p->xml++;
					MINIXML_ENSURE_MORE;
				}
			}
			else
			{
				attvalue = p->xml;
				attvaluelen = 0;
				while(   !IS_WHITE_SPACE(*p->xml)
					  && *p->xml != '>' && *p->xml != '/')
				{
					attvaluelen++; p->xml++;
					MINIXML_ENSURE_MORE;
				}
			}
			/*printf("%.*s='%.*s'\n",
			       attnamelen, attname, attvaluelen, attvalue);*/
			if(p->attfunc && (rc = p->attfunc(
					p->data, ns, nslen, attname, attnamelen,
					attvalue, attvaluelen)))
				return rc;
		}
		p->xml++;
	}
	return -1;
}

/* parseelt parse the xml stream and
 * call the callback functions when needed... */
int parseelt(struct xmlparser * p)
{
	int rc;
	unsigned i, nslen, comment;
	const char * elementname, * ns;
	comment = 0;
	while(p->xml < (p->xmlend - 1))
	{
		if (comment) {
			if (p->xml < (p->xmlend - 3) && !strncmp("-->", p->xml, 3)) {
				comment = 0;
			} else {
				p->xml ++;
				continue;
			}
		}
		if((p->xml)[0]=='<' && (p->xml)[1]!='?')
		{
			nslen = 0;
			ns = NULL;
			i = 0; elementname = ++p->xml;
			while( !IS_WHITE_SPACE(*p->xml)
				  && (*p->xml!='>') && (*p->xml!='/')
				 )
			{
				i++; p->xml++;
				if (i == 3 && !strncmp("!--", elementname, 3)) {
					comment = 1;
					break;
				}
				MINIXML_ENSURE_MORE;
				/* to ignore namespace : */
				if(*p->xml==':')
				{
					ns = elementname;
					nslen = i;
					i = 0;
					elementname = ++p->xml;
				}
			}
			if (comment) continue;
			if(i>0)
			{
				if(p->starteltfunc && (rc = p->starteltfunc(
						p->data, ns, nslen, elementname, i)))
					return rc;
				if((rc = parseatt(p)))
					return rc;
				if(*p->xml!='/')
				{
					const char * data;
					i = 0; data = ++p->xml;
					MINIXML_ENSURE_MORE;
					while( IS_WHITE_SPACE(*p->xml) )
					{
						p->xml++;
						MINIXML_ENSURE_MORE;
					}
					while(*p->xml!='<')
					{
						i++; p->xml++;
						MINIXML_ENSURE_MORE;
					}
					if(p->xml<p->xmlend && *(p->xml+1)=='/' && i >= 0 &&
							p->datafunc && (rc = p->datafunc(p->data, data, i)))
						return rc;
				} else {
					if(*(++p->xml) == '>') {
						if(p->endeltfunc && (rc = p->endeltfunc(
								p->data, ns, nslen, elementname, i)))
							return rc;
					} else {
						return -1;
					}
				}
			}
			else if(*p->xml == '/')
			{
				i = 0; elementname = ++p->xml;
				MINIXML_ENSURE_MORE;
				while((*p->xml != '>'))
				{
					i++; p->xml++;
					MINIXML_ENSURE_MORE;
					if(*p->xml==':')
					{
						ns = elementname;
						nslen = i;
						i = 0;
						elementname = ++p->xml;
					}
				}
				if(p->endeltfunc && (rc = p->endeltfunc(
						p->data, ns, nslen, elementname, i)))
					return rc;
				p->xml++;
			}
		}
		else
		{
			p->xml++;
		}
	}
	return 0;
}

/* the parser must be initialized before calling this function */
int parsexml(struct xmlparser * parser)
{
	parser->xml = parser->xmlstart;
	parser->xmlend = parser->xmlstart + parser->xmlsize;
	return parseelt(parser);
}


