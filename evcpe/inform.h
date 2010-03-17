// $Id$

#ifndef EVCPE_INFORM_H_
#define EVCPE_INFORM_H_

#include <time.h>

#include "data.h"

struct evcpe_inform {
	struct evcpe_device_id device_id;
	struct evcpe_event_list event;
	unsigned int max_envelopes;
	char current_time[26];
	unsigned int retry_count;
	struct evcpe_param_value_list parameter_list;
};

struct evcpe_inform *evcpe_inform_new(void);

void evcpe_inform_free(struct evcpe_inform *inform);

int evcpe_inform_add_event(struct evcpe_inform *inform,
		const char *event_code, const char *command_key);

struct evcpe_inform_response {
	unsigned int max_envelopes;
};

struct evcpe_inform_response *evcpe_inform_response_new(void);

void evcpe_inform_response_free(struct evcpe_inform_response *inform);

#endif /* EVCPE_INFORM_H_ */
