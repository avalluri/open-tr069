// $Id$

#include "request.h"

struct evcpe_request *evcpe_inform_new()
{
	struct evcpe_request *req;

	if (!(req = calloc(1, sizeof(struct evcpe_request)))) {
		evcpe_error(__func__, "failed to calloc evcpe_request");
		return NULL;
	}
	req->type = EVCPE_INFORM;
	return req;
}

void evcpe_request_free(struct evcpe_request *req)
{
	if (!req) return;

	free(req);
}
