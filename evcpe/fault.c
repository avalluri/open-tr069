// $Id$

#include <stdlib.h>

#include "log.h"

#include "fault.h"

struct evcpe_fault *evcpe_fault_new(void)
{
	struct evcpe_fault *fault;
	evcpe_debug(__func__, "constructing evcpe_fault");
	if (!(fault = calloc(1, sizeof(struct evcpe_fault)))) {
		evcpe_error(__func__, "failed to calloc evcpe_fault");
		return NULL;
	}
	return fault;
}

void evcpe_fault_free(struct evcpe_fault *fault)
{
	if (!fault) return;
	evcpe_debug(__func__, "destructing evcpe_fault");
	free(fault);
}
