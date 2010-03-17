// $Id$

#ifndef EVCPE_DELETE_OBJECT_H_
#define EVCPE_DELETE_OBJECT_H_

struct evcpe_delete_object {
	char object_name[257];
	char parameter_key[33];
};

struct evcpe_delete_object *evcpe_delete_object_new(void);

void evcpe_delete_object_free(struct evcpe_delete_object *method);

#endif /* EVCPE_DELETE_OBJECT_H_ */
