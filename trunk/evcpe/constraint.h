// $Id: constraint.h 124 2009-01-11 11:50:11Z cedric $

#ifndef EVCPE_CONSTRAINT_H_
#define EVCPE_CONSTRAINT_H_

#include <sys/types.h>
#include <sys/queue.h>

enum evcpe_constraint_type {
	EVCPE_CONSTRAINT_NONE,
	EVCPE_CONSTRAINT_ATTR,
	EVCPE_CONSTRAINT_SIZE,
	EVCPE_CONSTRAINT_ENUM,
	EVCPE_CONSTRAINT_MIN,
	EVCPE_CONSTRAINT_MAX,
	EVCPE_CONSTRAINT_RANGE
};

struct evcpe_constraint_enum {
	char *string;
	TAILQ_ENTRY(evcpe_constraint_enum) entry;
};

TAILQ_HEAD(evcpe_constraint_enums, evcpe_constraint_enum);

struct evcpe_constraint {
	enum evcpe_constraint_type type;
	union {
		unsigned size;
		struct evcpe_constraint_enums enums;
		struct {
			long min;
			long max;
		} range;
		char *attr;
	} value;
};

int evcpe_constraint_enums_add(struct evcpe_constraint_enums *enums,
		const char *string, unsigned len);

void evcpe_constraint_enums_clear(struct evcpe_constraint_enums *enums);

int evcpe_constraint_set_min(struct evcpe_constraint *cons,
		const char *min, unsigned len);

int evcpe_constraint_set_max(struct evcpe_constraint *cons,
		const char *max, unsigned len);

int evcpe_constraint_set_range(struct evcpe_constraint *cons,
		const char *min, unsigned minlen, const char *max, unsigned maxlen);

int evcpe_constraint_set_attr(struct evcpe_constraint *cons,
		const char *value, size_t len);

#endif /* EVCPE_CONSTRAINT_H_ */
