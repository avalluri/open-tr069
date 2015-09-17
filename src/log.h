// $Id$
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EVCPE_LOG_H_
#define EVCPE_LOG_H_

#include <stdarg.h>
#include <sys/queue.h>

#include <evcpe.h>

enum evcpe_log_level {
	EVCPE_LOG_TRACE,
	EVCPE_LOG_DEBUG,
	EVCPE_LOG_INFO,
	EVCPE_LOG_WARN,
	EVCPE_LOG_ERROR,
	EVCPE_LOG_FATAL
};

typedef void (*evcpe_logger_cb)(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap, void *cbarg);

struct evcpe_logger {
	const char *name;
	enum evcpe_log_level min;
	enum evcpe_log_level max;
	const char *prefix;
	unsigned len;
	evcpe_logger_cb cb;
	void *cbarg;
	TAILQ_ENTRY(evcpe_logger) entry;
};

TAILQ_HEAD(evcpe_loggers, evcpe_logger);

const char *evcpe_log_level_to_str(enum evcpe_log_level level);

void evcpe_log(enum evcpe_log_level level, const char *func,
		const char *fmt, ...) EVCPE_CHKFMT(3,4);

void evcpe_vlog(enum evcpe_log_level level, const char *func, const char *fmt,
		va_list ap);

void evcpe_file_logger(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap, void *cbarg);

int evcpe_add_logger(const char *name, enum evcpe_log_level min,
		enum evcpe_log_level max, const char *prefix, evcpe_logger_cb logger,
		void *cbarg);

int evcpe_remove_logger(const char *name);

int evcpe_set_log_level(enum evcpe_log_level min, enum evcpe_log_level max);

void evcpe_set_log_out(FILE *out, FILE *err);

void evcpe_trace(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_debug(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_info(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_warn(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_error(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

void evcpe_fatal(const char *func, const char *fmt, ...) EVCPE_CHKFMT(2,3);

#define ERROR(...) evcpe_error(__func__, __VA_ARGS__)
#define WARN(...)  evcpe_warn (__func__, __VA_ARGS__)
#define FATAL(...) evcpe_fatal(__func__, __VA_ARGS__)
#define INFO(...)  evcpe_info(__func__, __VA_ARGS__)
#define DEBUG(...) evcpe_debug(__func__, __VA_ARGS__)
#define TRACE(...) evcpe_trace(__func__, __VA_ARGS__)


#endif /* EVCPE_LOG_H_ */
