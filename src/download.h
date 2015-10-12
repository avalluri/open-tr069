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

#ifndef EVCPE_DOWNLOAD_H_
#define EVCPE_DOWNLOAD_H_

#include <time.h>
#include <event.h>

#include "url.h"

typedef enum _FileType {
	EVCPE_FILE_TYPE_UNKNOWN,
	EVCPE_FILE_TYPE_FIRMWARE_UPGRADE = 1,
	EVCPE_FILE_TYPE_WEB_CONTENT,
	EVCPE_FILE_TYPE_VENDOR_CONFIGURATION_FILE,
	EVCPE_FILE_TYPE_TONE_FILE,
	EVCPE_FILE_TYPE_RINGER_FILE,
	EVCPE_FILE_TYPE_VENDOR_SPECIFIC,
	EVCPE_FILE_TYPE_MAX
} evcpe_file_type_t;

typedef struct _evcpe_download
{
	char command_key[32];
	evcpe_url url;
	evcpe_file_type_t file_type;
	char username[256];
	char password[256];
	unsigned file_size;
	char target_filename[256];
	unsigned delay;
	evcpe_url success_url;
	evcpe_url failure_url;
} evcpe_download;

evcpe_download* evcpe_download_new();

void evcpe_download_free(evcpe_download* req);


typedef struct _evcpe_download_response
{
	int status;
	struct tm* start_time;
	struct tm* end_time;
} evcpe_download_response;

evcpe_download_response* evcpe_download_response_new();

void evcpe_download_response_free(evcpe_download_response* resp);

int evcpe_download_response_to_xml(evcpe_download_response* resp,
		struct evbuffer* buf);

typedef enum _window_mode {
	EVCPE_WINDOW_MODE_UNKNOWN,
	EVCPE_WINDOW_MODE_AT_ANY_TIME = 1,
	EVCPE_WINDOW_MODE_IMMEDIATELY,
	EVCPE_WINDOW_WHEN_IDLE,
	EVCPE_WINDOW_MODE_CONFIRMATION_NEEDED
} evcpe_window_mode_t;

typedef struct _time_window
{
	unsigned start;
	unsigned end;
	evcpe_window_mode_t mode;
	char message[256];
	int  max_retries;
} evcpe_download_time_window;

typedef struct _schedule_download
{
	evcpe_download* basic_info;
	evcpe_download_time_window *window;
} evcpe_schedule_download;

#endif /* EVCPE_DOWNLOAD_H_ */
