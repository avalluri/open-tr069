// $Id$

#include <time.h>
#include <errno.h>

#include "log.h"

#include "accessor.h"

int evcpe_get_curtime(struct evcpe_attr *attr,
		const char **value, unsigned int *len)
{
	char buf[32];
	time_t curtime;

	evcpe_debug(__func__, "get current time");
	// TODO: timezone
	if ((curtime = time(NULL)) == ((time_t)-1)) {
		evcpe_error(__func__, "failed to get time");
		return errno;
	}
 	*len = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&curtime));
	*value = buf;
	return 0;
}

int evcpe_set_curtime(struct evcpe_attr *attr,
		const char *buffer, unsigned int len)
{
	evcpe_error(__func__, "not implemented");
	return -1;
}
