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

#include <signal.h>
#include <mcheck.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <evcpe.h>
#include <evdns.h>

#include "cpe.h"
#include "class_xml.h"
#include "obj_xml.h"
#include "persister.h"
#include "evcpe-config.h"

static struct {
	struct event_base *evbase;
	struct evcpe *cpe;
	struct evcpe_class *cls;
	struct evcpe_obj *obj;
	struct evcpe_persister *persist;
	struct evcpe_repo *repo;
} this;

static void sig_handler(int signum);

static void error_cb(struct evcpe *cpe,
		enum evcpe_error_type type, int code, const char *reason, void *cbarg);

static int load_file(const char *filename, struct evbuffer *buffer);

static int shutdown_cpe(int code);

static void help(FILE *stream)
{
	fprintf(stream, "evcpe, version "EVCPE_VERSION"\n\n"
			"Usage: evcpe [options]\n\n"
			" Options are:\n"
			"  -m REPO_MODEL\tmodel of TR-069 repository\n"
			"  -d REPO_DATA\tdata of TR-069 repository\n"
			"  -v\t\tdisplay more logs\n"
			"  -s\t\tsuppress logs\n"
			"\n");
}

void error_cb(struct evcpe *cpe,
		enum evcpe_error_type type, int code, const char *reason, void *cbarg)
{
	evcpe_error(__func__, "type: %d, code: %d, reason: %s", type, code, reason);
	   shutdown_cpe(code);
}

int main(int argc, char **argv)
{
	int c, rc, bootstrap;
	enum evcpe_log_level level;
	struct sigaction action;
	const char *repo_model, *repo_data;
	struct evbuffer *buffer = NULL;

	repo_model = repo_data = NULL;
	level = EVCPE_LOG_INFO;
	while ((c = getopt(argc, argv, "m:d:sv")) != -1)
		switch (c) {
		case 'm':
			repo_model = optarg;
			break;
		case 'd':
			repo_data = optarg;
			break;
		case 's':
			level ++;
			break;
		case 'v':
			if (level > EVCPE_LOG_TRACE)
				level --;
			break;
		}
	argv += optind;
	argc -= optind;

	if (argc == 0 ) {
		help(stderr);
		exit(EINVAL);
	} else if (!strcmp("start", argv[0])) {
		bootstrap = 0;
	} else if (!strcmp("bootstrap", argv[0])) {
		bootstrap = 1;
	} else {
		help(stderr);
		exit(EINVAL);
	}

	if (!repo_model || !repo_data) {
		help(stderr);
		rc = EINVAL;
		goto finally;
	}

	if (level <= EVCPE_LOG_FATAL)
		evcpe_add_logger("stderr", level, EVCPE_LOG_FATAL,
				NULL, evcpe_file_logger, stdout);

	evcpe_info(__func__, "initializing evcpe");

	mtrace();

	if (!(buffer = evbuffer_new())) {
		evcpe_error(__func__, "failed to create evbuffer");
		rc = ENOMEM;
		goto finally;
	}

	bzero(&this, sizeof(this));

	if ((rc = load_file(repo_model, buffer))) {
		evcpe_error(__func__, "failed to load model: %s", repo_model);
		goto finally;
	}
	if (!(this.cls = evcpe_class_new(NULL))) {
		evcpe_error(__func__, "failed to create evcpe_class");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_class_from_xml(this.cls, buffer))) {
		evcpe_error(__func__, "failed to parse model: %s", repo_model);
		goto finally;
	}
	if (!(this.obj = evcpe_obj_new(this.cls, NULL))) {
		evcpe_error(__func__, "failed to create root object");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_obj_init(this.obj))) {
		evcpe_error(__func__, "failed to init root object");
		goto finally;
	}
	if ((rc = load_file(repo_data, buffer))) {
		evcpe_error(__func__, "failed to load data: %s", repo_data);
		goto finally;
	}
	if ((rc = evcpe_obj_from_xml(this.obj, buffer))) {
		evcpe_error(__func__, "failed to parse data: %s", repo_data);
		goto finally;
	}
	if (!(this.repo = evcpe_repo_new(this.obj))) {
		evcpe_error(__func__, "failed to create repo");
		rc = ENOMEM;
		goto finally;
	}
	if ((this.evbase = event_init()) == NULL) {
		evcpe_error(__func__, "failed to init event");
		rc = ENOMEM;
		goto finally;
	}
	if (!(this.persist = evcpe_persister_new(this.evbase))) {
		evcpe_error(__func__, "failed to create persister");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_persister_set(this.persist, this.repo, repo_data))) {
		evcpe_error(__func__, "failed to set persister");
		goto finally;
	}
	if ((rc = evdns_init())) {
		evcpe_error(__func__, "failed to initialize DNS");
		goto finally;
	}
	if ((this.cpe = evcpe_new(this.evbase,
			NULL, error_cb, NULL)) == NULL) {
		evcpe_error(__func__, "failed to create evcpe");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_set(this.cpe, this.repo))) {
		evcpe_error(__func__, "failed to set evcpe");
		goto finally;
	}
	evcpe_info(__func__, "configuring signal action");
	action.sa_handler = sig_handler;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;
	if ((rc = sigaction(SIGINT, &action, NULL)) != 0) {
		evcpe_error(__func__, "failed to configure signal action");
		goto finally;
	}

	evcpe_info(__func__, "starting evcpe");
	if ((rc = evcpe_start(this.cpe))) {
		evcpe_error(__func__, "failed to start evcpe");
		goto finally;
	}

	evcpe_info(__func__, "dispatching event base");
	if ((rc = event_dispatch()) != 0) {
		evcpe_error(__func__, "failed to dispatch event base");
		goto finally;
	}

finally:
	if (buffer) evbuffer_free(buffer);
	return rc;
}

int load_file(const char *filename, struct evbuffer *buffer)
{
	FILE *file;
	int rc, fd, len;

	if(!(file = fopen(filename, "r"))) {
		rc = errno;
		goto finally;
	}
	fd = fileno(file);
	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));
	do {
		len = evbuffer_read(buffer, fd, -1);
	} while(len > 0);

	rc = len < 0 ? -1 : 0;
	fclose(file);

finally:
	return rc;
}

void sig_handler(int signal)
{
	evcpe_info(__func__, "signal caught: %d", signal);
	if (signal == SIGINT)
		      shutdown_cpe(0);
}

int shutdown_cpe(int code)
{
	evcpe_info(__func__, "shuting down with code: %d (%s)", code, strerror(code));
	event_base_loopbreak(this.evbase);
	evcpe_free(this.cpe);
	evdns_shutdown(0);
	evcpe_persister_free(this.persist);
	event_base_free(this.evbase);
	evcpe_repo_free(this.repo);
	evcpe_obj_free(this.obj);
	evcpe_class_free(this.cls);

	muntrace();
	exit(code);
}
