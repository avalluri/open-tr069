// $Id$

#ifndef EVCPE_RESPONSE_H_
#define EVCPE_RESPONSE_H_

enum evcpe_response_type {
	EVCPE_RESPONSE_OK,
	EVCPE_RESPONSE_FAULT,
};

struct evcpe_response {
	char *session_id;
	enum evcpe_response_type type;
//	enum evcpe_method_type method_type;
	void *data;
};

#endif /* EVCPE_RESPONSE_H_ */
