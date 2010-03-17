// $Id$

#ifndef EVCPE_HANDLER_H_
#define EVCPE_HANDLER_H_

#include <sys/tree.h>

#include "request.h"
#include "error.h"

struct evcpe;

typedef void (*evcpe_request_cb)(struct evcpe_request *req, void *cbarg);

typedef void (*evcpe_error_cb)(struct evcpe *cpe,
		enum evcpe_error_type type, int code, const char *reason, void *cbarg);

struct evcpe_handler {
//	enum evcpe_request_type type;
	evcpe_request_cb cb;
	void *cbarg;
	RB_ENTRY(evcpe_handler) entry;
};

RB_HEAD(evcpe_handlers, evcpe_handler);

int evcpe_handler_cmp(struct evcpe_handler *a, struct evcpe_handler *b);

RB_PROTOTYPE(evcpe_handlers, evcpe_handler, entry, evcpe_handler_cmp);

#endif /* EVCPE_HANDLER_H_ */
