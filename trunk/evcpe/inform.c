// $Id$

#include <stdlib.h>

#include "log.h"

#include "inform.h"

struct evcpe_inform *evcpe_inform_new(void)
{
	struct evcpe_inform *inform;

	evcpe_debug(__func__, "constructing evcpe_inform");

	if (!(inform = calloc(1, sizeof(struct evcpe_inform)))) {
		evcpe_error(__func__, "failed to calloc evcpe_inform");
		return NULL;
	}
	inform->max_envelopes = 1;
	evcpe_event_list_init(&inform->event);
	evcpe_param_value_list_init(&inform->parameter_list);

	return inform;
}

void evcpe_inform_free(struct evcpe_inform *inform)
{
	if (!inform) return;

	evcpe_debug(__func__, "destructing evcpe_inform");

	evcpe_event_list_clear(&inform->event);
	evcpe_param_value_list_clear(&inform->parameter_list);
	free(inform);
}

struct evcpe_inform_response *evcpe_inform_response_new(void)
{
	struct evcpe_inform_response *method;

	evcpe_debug(__func__, "constructing evcpe_inform_response");

	if (!(method = calloc(1, sizeof(struct evcpe_inform_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_inform_response");
		return NULL;
	}
	return method;
}

void evcpe_inform_response_free(struct evcpe_inform_response *inform)
{
	if (!inform) return;
	evcpe_debug(__func__, "destructing evcpe_inform_response");
	free(inform);
}
