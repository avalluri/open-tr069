// $Id$

#ifndef EVCPE_GET_PARAM_NAMES_H_
#define EVCPE_GET_PARAM_NAMES_H_

#include "data.h"

struct evcpe_get_param_names {
	char parameter_path[257];
	int next_level;
};

struct evcpe_get_param_names *evcpe_get_param_names_new(void);

void evcpe_get_param_names_free(struct evcpe_get_param_names *method);

struct evcpe_get_param_names_response {
	struct evcpe_param_info_list parameter_list;
};

struct evcpe_get_param_names_response *evcpe_get_param_names_response_new(void);

void evcpe_get_param_names_response_free(
		struct evcpe_get_param_names_response *method);

#endif /* EVCPE_GET_PARAM_NAMES_H_ */
