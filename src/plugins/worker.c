/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of open-tr069
 *
 * Copyright (C) 2013-2015 Intel Corporation.
 *
 * Contact: Amarnath Valluri <amarnath.valluri@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <plugin.h>
#include <tqueue.h>
#include <log.h>

static int fd_out[2], fd_in[2];
static pid_t child_pid;
static tqueue* download_requests;

#define EVCPE_WORKER "/usr/bin/evcpe-worker"

typedef struct {
	evcpe_download* req;
	evcpe_download_state_change_cb_t cb;
	void* data;
} download_info;

static
download_info* download_info_new(const evcpe_download* req,
		evcpe_download_state_change_cb_t cb, void* data) {
	download_info* info = calloc(1, sizeof(*info));

	if (!info) return NULL;

	info->req = (evcpe_download*)req;
	info->cb = cb;
	info->data = data;

	return info;
}

static
void download_info_free(void *info) {
	free(info);
}

static
int _init(evcpe_plugin* self)
{
	DEBUG("Initializing worker plugin");
	if (pipe(fd_out) < 0) {
		ERROR("Failed to create in pipe: %s", strerror(errno));
		return -1;
	}
	if (pipe(fd_in) < 0) {
		close(fd_out[0]);
		close(fd_out[1]);
		ERROR("Failed to create out pipe: %s", strerror(errno));
		return -1;
	}

	if ((child_pid = fork()) < 0) {
		ERROR("Failed to fork: %s", strerror(errno));
		close(fd_out[0]); close(fd_out[1]);
		close(fd_in[0]); close(fd_in[1]);
		return -1;
	}

	/* child process */
	if (child_pid == 0) {
		char* argv[] = { EVCPE_WORKER, NULL };

		dup2(fd_out[1], STDOUT_FILENO);
		close(fd_out[0]);
		close(fd_out[1]);

		dup2(fd_in[0], STDIN_FILENO);
		close(fd_in[0]);
		close(fd_in[1]);

		execvp(argv[0], argv);
		exit(0);
	} else {
		close(fd_out[1]);
		close(fd_in[0]);
		return 0;
	}
}

static
void _uninit(evcpe_plugin* self) {
	int status;

	if (download_requests) tqueue_free(download_requests);

	if (kill(child_pid, SIGTERM) <0) {
		ERROR("Failed to send SIGTERM to child process: %d", child_pid);
		return;
	}

	while(wait(&status) != child_pid) {
	}

	DEBUG("worker terminated");
	close(fd_out[0]);
	close(fd_in[1]);
}

static
int _handle_download(evcpe_plugin* self, const evcpe_download* details,
		evcpe_download_state_change_cb_t cb, void* cb_data) {
	if (!download_requests)
		download_requests = tqueue_new(NULL, download_info_free);
	tqueue_insert(download_requests, download_info_new(details, cb, cb_data));
	//TODO: Send the download details(json?) to worker
	return 0;
}

static
int _handle_reboot(evcpe_plugin* self) {
	//TODO: Inform(json?) worker about reboot.
	return 0;
}

EVCPE_PLUGIN_REGISTER("worker", "0.0.1", _init, _uninit, NULL, NULL,
		_handle_download, _handle_reboot);
