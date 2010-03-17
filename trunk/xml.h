// $Id$

#ifndef EVCPE_XML_H_
#define EVCPE_XML_H_

#include <time.h>
#include <stdarg.h>
#include <sys/queue.h>

#include <event.h>

struct evcpe_xml_attr {
	const char *ns;
	unsigned ns_len;
	const char *name;
	unsigned name_len;
	const char *value;
	unsigned value_len;
	TAILQ_ENTRY(evcpe_xml_attr) entry;
};

TAILQ_HEAD(evcpe_xml_attrs, evcpe_xml_attr);

struct evcpe_xml_parser {
	struct evbuffer *buffer;
	const char *data;
	unsigned len;
	unsigned offset;
	const char *ns;
	unsigned ns_len;
	const char *name;
	unsigned name_len;
	struct evcpe_xml_attrs attrs;
};

int evcpe_xml_attrs_add(struct evcpe_xml_attrs *attrs,
		const char *ns, unsigned ns_len, const char *name, unsigned name_len,
		const char *value, unsigned value_len);

void evcpe_xml_attrs_clear(struct evcpe_xml_attrs *attrs);

int evcpe_xml_expect_elm(struct evcpe_xml_parser *parser);

int evcpe_xml_add_indent(struct evbuffer *buffer,
		const char *indent, unsigned int count);

inline int evcpe_xml_add_string(struct evbuffer *buffer,
		const char *node, const char *string);

inline int evcpe_xml_add_int(struct evbuffer *buffer,
		const char *node, int value);

int evcpe_xml_add_datetime(struct evbuffer *buffer,
		const char *node, struct tm *datetime);

int evcpe_xml_add_xsd_string(struct evbuffer *buffer,
		const char *node, const char *string);

int evcpe_xml_add_xsd_boolean(struct evbuffer *buffer,
		const char *node, int value);

int evcpe_xml_add_xsd_int(struct evbuffer *buffer,
		const char *node, int value);

int evcpe_xml_add_xsd_unsigned_int(struct evbuffer *buffer,
		const char *node, unsigned int value);

int evcpe_xml_add_xsd_base64(struct evbuffer *buffer,
		const char *node, u_char *data, unsigned len);


#endif /* EVCPE_XML_H_ */
