// $Id: type.h 124 2009-01-11 11:50:11Z cedric $

#ifndef EVCPE_TYPE_H_
#define EVCPE_TYPE_H_

#include <sys/types.h>

#include "constraint.h"

enum evcpe_type {
	EVCPE_TYPE_UNKNOWN,
	EVCPE_TYPE_OBJECT,
	EVCPE_TYPE_MULTIPLE,
	EVCPE_TYPE_STRING,
	EVCPE_TYPE_INT,
	EVCPE_TYPE_UNSIGNED_INT,
	EVCPE_TYPE_BOOLEAN,
	EVCPE_TYPE_DATETIME,
	EVCPE_TYPE_BASE64
};

const char *evcpe_type_to_str(enum evcpe_type type);

int evcpe_type_validate(enum evcpe_type type, const char *value, unsigned len,
		struct evcpe_constraint *cons);

#endif /* EVCPE_TYPE_H_ */
