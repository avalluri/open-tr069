// $Id$

#ifndef EVCPE_ACCESSOR_H_
#define EVCPE_ACCESSOR_H_

#include "attr_schema.h"

int evcpe_get_curtime(struct evcpe_attr *attr,
		const char **value, unsigned int *len);

int evcpe_set_curtime(struct evcpe_attr *attr,
		const char *buffer, unsigned int len);

#endif /* EVCPE_ACCESSOR_H_ */
