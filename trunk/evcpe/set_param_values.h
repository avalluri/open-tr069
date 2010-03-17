// $Id$

#ifndef EVCPE_SET_PARAM_VALUES_H_
#define EVCPE_SET_PARAM_VALUES_H_

#include "data.h"

struct evcpe_set_param_values {
	struct evcpe_set_param_value_list parameter_list;
};

struct evcpe_set_param_values *evcpe_set_param_values_new(void);

void evcpe_set_param_values_free(struct evcpe_set_param_values *method);

struct evcpe_set_param_values_response {
	int status;
};

struct evcpe_set_param_values_response *evcpe_set_param_values_response_new(void);

void evcpe_set_param_values_response_free(
		struct evcpe_set_param_values_response *method);

#endif /* EVCPE_SET_PARAM_VALUES_H_ */
