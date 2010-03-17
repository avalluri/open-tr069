// $Id$

#ifndef EVCPE_GET_PARAM_VALUES_H_
#define EVCPE_GET_PARAM_VALUES_H_

#include "data.h"

struct evcpe_get_param_values {
	struct evcpe_param_name_list parameter_names;
};

struct evcpe_get_param_values *evcpe_get_param_values_new(void);

void evcpe_get_param_values_free(struct evcpe_get_param_values *method);

struct evcpe_get_param_values_response {
	struct evcpe_param_value_list parameter_list;
};

struct evcpe_get_param_values_response *evcpe_get_param_values_response_new(void);

void evcpe_get_param_values_response_free(
		struct evcpe_get_param_values_response *method);

#endif /* EVCPE_GET_PARAM_VALUES_H_ */
