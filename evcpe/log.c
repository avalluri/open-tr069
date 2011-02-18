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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"

#include "log.h"

static struct evcpe_loggers loggers = TAILQ_HEAD_INITIALIZER(loggers);

const char *evcpe_log_level_to_str(enum evcpe_log_level level)
{
	switch(level) {
	case EVCPE_LOG_TRACE:
		return "TRACE";
	case EVCPE_LOG_DEBUG:
		return "DEBUG";
	case EVCPE_LOG_INFO:
		return "INFO";
	case EVCPE_LOG_WARN:
		return "WARN";
	case EVCPE_LOG_ERROR:
		return "ERROR";
	case EVCPE_LOG_FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}


int evcpe_add_logger(const char *name,
		enum evcpe_log_level min, enum evcpe_log_level max,
		const char *prefix, evcpe_logger cb, void *cbarg)
{
	struct evcpe_logger *logger;

	if (!name || max < min) return EINVAL;

	if (!(logger = calloc(1, sizeof(struct evcpe_logger))))
		return ENOMEM;
	logger->name = name;
	logger->prefix = prefix;
	if (prefix) logger->len = strlen(prefix);
	logger->min = min;
	logger->max = max;
	logger->cb = cb;
	logger->cbarg = cbarg;
	TAILQ_INSERT_TAIL(&loggers, logger, entry);

	return 0;
}

int evcpe_remove_logger(const char *name)
{
	struct evcpe_logger *iter, *match;

	match = NULL;
	TAILQ_FOREACH(iter, &loggers, entry) {
		if (!strcmp(name, iter->name)) {
			match = iter;
			break;
		}
	}
	if (match) {
		TAILQ_REMOVE(&loggers, match, entry);
		free(match);
		return 0;
	} else {
		return EINVAL;
	}
}

void evcpe_file_logger(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap, void *cbarg)
{
	time_t curtime;
	struct tm *loctime;
	char timestr[64];
	FILE *stream = cbarg;

	curtime = time(NULL);
	loctime = localtime(&curtime);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", loctime);

	fprintf(stream, "[%s] %s\t%s - ", timestr,
			evcpe_log_level_to_str(level), func);
	vfprintf(stream, fmt, ap);

	if (fmt[strlen(fmt) - 1] != '\n')
		fputc('\n', stream);
}

void evcpe_vlog(enum evcpe_log_level level, const char *func,
		const char *fmt, va_list ap)
{
	struct evcpe_logger *logger;

	TAILQ_FOREACH(logger, &loggers, entry) {
		if (!logger->cb)
			continue;
		if (level < logger->min || level > logger->max)
			continue;
		if (logger->prefix && strncmp(logger->prefix, func, logger->len))
			continue;
		(*logger->cb)(level, func, fmt, ap, logger->cbarg);
	}
}

void evcpe_log(enum evcpe_log_level level, const char *func,
		const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(level, func, fmt, ap);
	va_end(ap);
}

void evcpe_trace(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_TRACE, func, fmt, ap);
	va_end(ap);
}

void evcpe_debug(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_DEBUG, func, fmt, ap);
	va_end(ap);
}

void evcpe_info(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_INFO, func, fmt, ap);
	va_end(ap);
}

void evcpe_warn(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_WARN, func, fmt, ap);
	va_end(ap);
}

void evcpe_error(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_ERROR, func, fmt, ap);
	va_end(ap);
}

void evcpe_fatal(const char *func, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	evcpe_vlog(EVCPE_LOG_FATAL, func, fmt, ap);
	va_end(ap);
}
