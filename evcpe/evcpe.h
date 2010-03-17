// $Id$

#ifndef EVCPE_H_
#define EVCPE_H_

#ifdef __GNUC__
#define EVCPE_CHKFMT(a,b) __attribute__((format(printf, a, b)))
#else
#define EVCPE_CHKFMT(a,b)
#endif

#include <sys/types.h>
#include <stdio.h>

#include <evhttp.h>

#define EVCPE_IANA_CWMP_PORT 7547

enum evcpe_log_level {
	EVCPE_LOG_TRACE,
	EVCPE_LOG_DEBUG,
	EVCPE_LOG_INFO,
	EVCPE_LOG_WARN,
	EVCPE_LOG_ERROR,
	EVCPE_LOG_FATAL
};

typedef void (*evcpe_logger)(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap, void *cbarg);

void evcpe_file_logger(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap, void *cbarg);

int evcpe_add_logger(const char *name,
		enum evcpe_log_level min, enum evcpe_log_level max,
		const char *prefix, evcpe_logger logger, void *cbarg);

int evcpe_remove_logger(const char *name);

int evcpe_set_log_level(enum evcpe_log_level min, enum evcpe_log_level max);

void evcpe_set_log_out(FILE *out, FILE *err);

void evcpe_trace(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_debug(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_info(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_warn(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_error(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_fatal(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

#endif /* EVCPE_H_ */
