// $Id$

#ifndef EVCPE_BASE_H_
#define EVCPE_BASE_H_

#include <sys/tree.h>

struct evcpe_obj {
	RB_ENTRY(evcpe_obj) next;
};

struct evcpe_base {

};

struct evcpe_base *evcpe_base_new(void);

void evcpe_base_free(struct evcpe_base *base);

#endif /* EVCPE_OBJBASE_H_ */
