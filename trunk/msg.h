// $Id$

#ifndef EVCPE_MSG_H_
#define EVCPE_MSG_H_

#include <sys/queue.h>

#include <event.h>

#include "minixml.h"
#include "xmlns.h"
#include "xml.h"
#include "method.h"
#include "xml_stack.h"

enum evcpe_msg_type {
	EVCPE_MSG_UNKNOWN,
	EVCPE_MSG_REQUEST,
	EVCPE_MSG_RESPONSE,
	EVCPE_MSG_FAULT,
};

const char *evcpe_msg_type_to_str(enum evcpe_msg_type type);

struct evcpe_msg {
	char *session;
	unsigned int major;
	unsigned int minor;
	enum evcpe_msg_type type;
	enum evcpe_method_type method_type;
	void *data;
	int hold_requests;
	int no_more_requests; // CWMP 1.0
	TAILQ_ENTRY(evcpe_msg) entry;
};

TAILQ_HEAD(evcpe_msg_queue, evcpe_msg);

struct evcpe_msg_parser {
	struct xmlparser xml;
	struct evcpe_xmlns_table xmlns;
	struct evcpe_xml_stack stack;
	struct evcpe_msg *msg;
	void *list_item;
};

struct evcpe_msg *evcpe_msg_new(void);

void evcpe_msg_free(struct evcpe_msg *msg);

void evcpe_msg_queue_clear(struct evcpe_msg_queue *queue);

#endif /* EVCPE_MSG_H_ */
