// $Id: type.c 163 2009-01-25 10:25:51Z cedric $

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "util.h"

#include "type.h"

const char *evcpe_type_to_str(enum evcpe_type type)
{
	switch (type) {
	case EVCPE_TYPE_OBJECT:
		return "object";
	case EVCPE_TYPE_MULTIPLE:
		return "multipleObject";
	case EVCPE_TYPE_STRING:
		return "string";
	case EVCPE_TYPE_INT:
		return "int";
	case EVCPE_TYPE_UNSIGNED_INT:
		return "unsignedInt";
	case EVCPE_TYPE_BOOLEAN:
		return "boolean";
	case EVCPE_TYPE_DATETIME:
		return "dateTime";
	case EVCPE_TYPE_BASE64:
		return "base64";
	case EVCPE_TYPE_UNKNOWN:
	default:
		return "unknown";
	}
}

int evcpe_type_validate(enum evcpe_type type, const char *value, unsigned len,
		struct evcpe_constraint *cons)
{
	int rc;
	long val;
	char *dup;
	struct tm tm;

	evcpe_debug(__func__, "validating value of type: %s",
			evcpe_type_to_str(type));

	switch(type) {
	case EVCPE_TYPE_STRING:
		break;
	case EVCPE_TYPE_BASE64:
		// TODO
		break;
	case EVCPE_TYPE_INT:
	case EVCPE_TYPE_UNSIGNED_INT:
	case EVCPE_TYPE_BOOLEAN:
		if ((rc = evcpe_atol(value, len, &val))) {
			evcpe_error(__func__, "failed to convert to "
					"integer: %.*s", len, value);
			goto finally;
		}
		if (type == EVCPE_TYPE_UNSIGNED_INT) {
			if (val < 0) {
				evcpe_error(__func__, "not a positive integer: %ld", val);
				goto finally;
			}
		} else if(type == EVCPE_TYPE_BOOLEAN) {
			if (val != 0 && val != 1) {
				evcpe_error(__func__, "boolean value should be "
						"either 0 or 1: %ld", val);
				goto finally;
			}
		}
		switch(cons->type) {
		case EVCPE_CONSTRAINT_NONE:
			break;
		case EVCPE_CONSTRAINT_MIN:
		case EVCPE_CONSTRAINT_MAX:
		case EVCPE_CONSTRAINT_RANGE:
			if (cons->type != EVCPE_CONSTRAINT_MAX &&
					val < cons->value.range.min) {
				evcpe_error(__func__, "value out of range: %ld < %ld",
						val, cons->value.range.min);
				rc = EINVAL;
				goto finally;
			}
			if (cons->type != EVCPE_CONSTRAINT_MIN &&
					val > cons->value.range.max) {
				evcpe_error(__func__, "value out of range: %ld > %ld",
						val, cons->value.range.max);
				rc = EINVAL;
				goto finally;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected constraint type: %d", cons->type);
			rc = EINVAL;
			goto finally;
		}
		break;
	case EVCPE_TYPE_DATETIME:
		if (!(dup = malloc(len + 1))) {
			evcpe_error(__func__, "failed to malloc: %d bytes", len + 1);
			rc = ENOMEM;
			goto finally;
		}
		memcpy(dup, value, len);
		dup[len] = '\0';
		if (!strptime(dup, "%Y-%m-%dT%H:%M:%S", &tm)
				&& !strptime(dup, "%Y-%m-%dT%H:%M:%S%z", &tm)) {
			evcpe_error(__func__, "failed to parse dateTime: %s", dup);
			free(dup);
			rc = EINVAL;
			goto finally;
		}
		free(dup);
		break;
	case EVCPE_TYPE_UNKNOWN:
	case EVCPE_TYPE_OBJECT:
	case EVCPE_TYPE_MULTIPLE:
	default:
		evcpe_error(__func__, "value is not applicable to "
				"type: %d", type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}
