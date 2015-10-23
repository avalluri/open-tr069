/*
 * evcpe_plugin.h
 *
 *  Created on: Sep 24, 2015
 *      Author: avalluri
 */

#ifndef EVCPE_PLUGIN_H_
#define EVCPE_PLUGIN_H_

#include <time.h>
#include <url.h>

typedef struct _evcpe_plugin evcpe_plugin;
typedef struct _evcpe_plugin_priv evcpe_plugin_priv;

#define UNKNOWN_TIME "0001-01-01T00:00:00Z"

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
	char command_key[33];
	evcpe_url* url;
	evcpe_file_type_t file_type;
	char file_type_str[65];
	char username[257];
	char password[257];
	unsigned file_size;
	char target_filename[257];
	unsigned delay;
	evcpe_url* success_url;
	evcpe_url* failure_url;
} evcpe_download;

typedef enum _evcpe_download_state {
	EVCPE_DOWNLOAD_FINISHED,
	EVCPE_DOWNLOAD_APPLIED,
	EVCPE_DOWNLOAD_FAILED
} evcpe_download_state_t;

typedef enum _evcpe_download_failure {
	EVCPE_DOWNLOAD_INVALID_ARGUMETNS, //= 9003,
	EVCPE_DOWNLOAD_NO_SPACE, // = 9010,
	EVCPE_DOWNLOAD_AUTH_FAILURE //= 9012
} evcpe_download_failure_t;

typedef union _evcpe_download_satate_info {
	evcpe_download_state_t state;
	struct timeinfo {
		struct tm start_time;
		struct tm end_time;
	} download_complete;
	evcpe_download_failure_t failure;
} evcpe_download_state_info;


typedef void (*download_state_change_cb_t)(evcpe_download* details,
		evcpe_download_state_info* info, void* userdata);

struct _evcpe_plugin
{
	const char *name;
	const char *version;
	evcpe_plugin_priv* priv;

	int  (*init)(evcpe_plugin* p);
	void (*uninit)(evcpe_plugin* p);
	int  (*set_parameter_value)(evcpe_plugin* p, const char* name,
			const char* value, unsigned len);
	int  (*get_paramter_value)(evcpe_plugin* p, const char* name,
			const char** value_out, unsigned* len_out);

	int (*download)(evcpe_plugin* p, evcpe_download *details,
			download_state_change_cb_t cb, void* userdata);

	int (*reboot)(evcpe_plugin* p);
};
/* API for Plugins */
void evcpe_plugin_emit_value_change(evcpe_plugin* p, const char* name,
		const char* value, unsigned value_len);

#define EVCPE_PLUGIN_REGISTER(\
                 _name, _version, _init, _spv, _gpv, _download, _reboot) \
	evcpe_plugin* evcpe_plugin_get_object() {   \
		static evcpe_plugin _p = {            \
			.name = _name,                      \
			.version = _version,                \
			.init = _init,                      \
			.uninit = _uninit,                  \
			.set_parameter_value = _spv,        \
			.get_paramter_value = _gpv,         \
      .download = _download,              \
      .reboot = _rboot                    \
		};                                    \
		return &_p;                           \
	}                                       \
	struct _unused_


#endif /* EVCPE_PLUGIN_H_ */
