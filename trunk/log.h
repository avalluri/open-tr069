// $Id$

#ifndef EVCPE_LOG_H_
#define EVCPE_LOG_H_

#include <stdarg.h>
#include <sys/queue.h>

#include <evcpe.h>

struct evcpe_logger {
	const char *name;
	enum evcpe_log_level min;
	enum evcpe_log_level max;
	const char *prefix;
	unsigned len;
	evcpe_logger cb;
	void *cbarg;
	TAILQ_ENTRY(evcpe_logger) entry;
};

TAILQ_HEAD(evcpe_loggers, evcpe_logger);

const char *evcpe_log_level_to_str(enum evcpe_log_level level);

void evcpe_log(enum evcpe_log_level level, const char *func,
		const char *fmt, ...) EVCPE_CHKFMT(3,4);

void evcpe_vlog(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap);

#endif /* EVCPE_LOG_H_ */
