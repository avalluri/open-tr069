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


#include <evdns.h>

#include "evcpe.h"
#include "class_xml.h"
#include "obj_xml.h"
#include "persister.h"
#include "evcpe-config.h"
#include "log.h"

static struct {
	struct event_base *evbase;
	struct evcpe *cpe;
	struct evcpe_class *cls;
	struct evcpe_obj *obj;
	struct evcpe_persister *persist;
	struct evcpe_repo *repo;
} this;

static
void help(FILE *stream)
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

static
int load_file(const char *filename, struct evbuffer *buffer)
{
	FILE *file;
	int rc, fd, len;

	if(!(file = fopen(filename, "r"))) {
		rc = errno;
		goto finally;
	}
	fd = fileno(file);
	evbuffer_drain(buffer, evbuffer_get_length(buffer));
	do {
		len = evbuffer_read(buffer, fd, -1);
	} while(len > 0);

	rc = len < 0 ? -1 : 0;
	fclose(file);

finally:
	return rc;
}

static
int shutdown_cpe(int code)
{
	INFO("shuting down with code: %d (%s)", code, strerror(code));
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

static
void sig_handler(int signal)
{
	INFO("signal caught: %d", signal);
	if (signal == SIGINT)
		shutdown_cpe(0);
}

static
void error_cb(struct evcpe *cpe, enum evcpe_error_type type, int code,
		const char *reason, void *cbarg)
{
	ERROR("type: %d, code: %d, reason: %s", type, code, reason);
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

	if (bootstrap) {
		//FIXME: notify '0 BOOTSTRAP' Event to ACS
	}

	if (!repo_model || !repo_data) {
		help(stderr);
		rc = EINVAL;
		goto finally;
	}

	if (level <= EVCPE_LOG_FATAL)
		evcpe_add_logger("stderr", level, EVCPE_LOG_FATAL,
				NULL, evcpe_file_logger, stdout);

	INFO("initializing evcpe");

	mtrace();

	if (!(buffer = evbuffer_new())) {
		ERROR("failed to create evbuffer");
		rc = ENOMEM;
		goto finally;
	}

	bzero(&this, sizeof(this));

	if ((rc = load_file(repo_model, buffer))) {
		ERROR("failed to load model: %s", repo_model);
		goto finally;
	}
	if (!(this.cls = evcpe_class_new(NULL))) {
		ERROR("failed to create evcpe_class");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_class_from_xml(this.cls, buffer))) {
		ERROR("failed to parse model: %s", repo_model);
		goto finally;
	}
	if (!(this.obj = evcpe_obj_new(this.cls, NULL))) {
		ERROR("failed to create root object");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_obj_init(this.obj))) {
		ERROR("failed to init root object");
		goto finally;
	}

	if ((rc = load_file(repo_data, buffer))) {
		ERROR("failed to load data: %s", repo_data);
		goto finally;
	}
	if ((rc = evcpe_obj_from_xml(this.obj, buffer))) {
		ERROR("failed to parse data: %s", repo_data);
		goto finally;
	}
	if (!(this.repo = evcpe_repo_new(this.obj))) {
		ERROR("failed to create repo");
		rc = ENOMEM;
		goto finally;
	}
	if ((this.evbase = event_init()) == NULL) {
		ERROR("failed to init event");
		rc = ENOMEM;
		goto finally;
	}
#if 1
	if (!(this.persist = evcpe_persister_new(this.evbase))) {
		ERROR("failed to create persister");
		rc = ENOMEM;
		goto finally;
	}

	if ((rc = evcpe_persister_set(this.persist, this.repo, repo_data))) {
		ERROR("failed to set persister");
		goto finally;
	}
#endif

	if ((this.cpe = evcpe_new(this.evbase, NULL, error_cb, NULL)) == NULL) {
		ERROR("failed to create evcpe");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_set(this.cpe, this.repo))) {
		ERROR("failed to set evcpe");
		goto finally;
	}
	INFO("configuring signal action");
	action.sa_handler = sig_handler;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;
	if ((rc = sigaction(SIGINT, &action, NULL)) != 0) {
		ERROR("failed to configure signal action");
		goto finally;
	}

	INFO("starting evcpe");
	if ((rc = evcpe_start(this.cpe))) {
		ERROR("failed to start evcpe");
		goto finally;
	}

	INFO("dispatching event base");
	if ((rc = event_dispatch()) != 0) {
		ERROR("failed to dispatch event base");
		goto finally;
	}

finally:
	if (buffer) evbuffer_free(buffer);
	return rc;
}

