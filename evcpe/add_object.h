// $Id$

#ifndef EVCPE_ADD_OBJECT_H_
#define EVCPE_ADD_OBJECT_H_

struct evcpe_add_object {
	char object_name[257];
	char parameter_key[33];
};

struct evcpe_add_object *evcpe_add_object_new(void);

void evcpe_add_object_free(struct evcpe_add_object *method);

struct evcpe_add_object_response {
	unsigned int instance_number;
	int status;
};

struct evcpe_add_object_response *evcpe_add_object_response_new(void);

void evcpe_add_object_response_free(struct evcpe_add_object_response *method);

#endif /* EVCPE_ADD_OBJECT_H_ */
