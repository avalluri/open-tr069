// $Id$

#ifndef EVCPE_MSG_XML_H_
#define EVCPE_MSG_XML_H_

#include "msg.h"

int evcpe_msg_from_xml(struct evcpe_msg *msg, struct evbuffer *buffer);

int evcpe_msg_to_xml(struct evcpe_msg *msg, struct evbuffer *buffer);

#endif /* EVCPE_MSG_XML_H_ */
