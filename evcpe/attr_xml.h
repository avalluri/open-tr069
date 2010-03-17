// $Id$

#ifndef EVCPE_REPO_XML_H_
#define EVCPE_REPO_XML_H_

#include <event.h>

#include "repo.h"

int evcpe_attr_to_xml(struct evcpe_attr *attr,
		unsigned int indent, struct evbuffer *buffer);

int evcpe_attr_to_xml_param_names(struct evcpe_attr *attr, int next_level,
		struct evbuffer *buffer);

int evcpe_attr_count_xml_param_names(struct evcpe_attr *attr, int next_level,
		unsigned int *count);

#endif /* REPO_XML_H_ */
