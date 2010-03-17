// $Id$

#ifndef EVCPE_GET_PARAM_ATTRS_H_
#define EVCPE_GET_PARAM_ATTRS_H_

#include "data.h"

struct evcpe_get_param_attrs {
	struct evcpe_param_name_list parameter_names;
};

struct evcpe_get_param_attrs *evcpe_get_param_attrs_new(void);

void evcpe_get_param_attrs_free(struct evcpe_get_param_attrs *method);

struct evcpe_get_param_attrs_response {
	struct evcpe_param_attr_list parameter_list;
};

struct evcpe_get_param_attrs_response *evcpe_get_param_attrs_response_new(void);

void evcpe_get_param_attrs_response_free(
		struct evcpe_get_param_attrs_response *method);

#endif /* EVCPE_GET_PARAM_ATTRS_H_ */
