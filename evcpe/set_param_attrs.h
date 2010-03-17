// $Id$

#ifndef EVCPE_SET_PARAM_ATTRS_H_
#define EVCPE_SET_PARAM_ATTRS_H_

#include "data.h"

struct evcpe_set_param_attrs {
	struct evcpe_set_param_attr_list parameter_list;
};

struct evcpe_set_param_attrs *evcpe_set_param_attrs_new(void);

void evcpe_set_param_attrs_free(struct evcpe_set_param_attrs *method);

#endif /* EVCPE_SET_PARAM_ATTRS_H_ */
