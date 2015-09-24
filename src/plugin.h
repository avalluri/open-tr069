/*
 * evcpe_plugin.h
 *
 *  Created on: Sep 24, 2015
 *      Author: avalluri
 */

#ifndef EVCPE_PLUGIN_H_
#define EVCPE_PLUGIN_H_

struct evcpe_set_paramter_attributes {
	char name[256];
	int notification_change;
	int notification[6];
	int access_list_change;
	char *access_list[64];
};

typedef struct _evcpe_plugin evcpe_plugin;

struct _evcpe_plugin
{
	unsigned ref_count;
	void* handle;
	const char *name;
	const char *version;

	int  (*init)(evcpe_plugin* p);
	void (*uninit)(evcpe_plugin* p);
	int  (*set_parameter_value)(evcpe_plugin* p, const char* name,
			const char* value, unsigned len);
	int (*get_paramter_value)(evcpe_plugin* p, const char* name,
			const char** value_out, unsigned* len_out);
};

int evcpe_plugin_load(const char* name, evcpe_plugin** plugin_out);
void evcpe_plugin_unload(evcpe_plugin* self);
evcpe_plugin* evcpe_plugin_ref(evcpe_plugin* self);
void evcpe_plugin_unref(evcpe_plugin* self);

#define EVCPE_PLUGIN_REGISTER(\
                      _name, _version, _init, _spv, _gpv) \
	evcpe_plugin* evcpe_plugin_get_object(void* h) {      \
		static evcpe_plugin _p = {                        \
			.ref_count = 1,                               \
			.handle = h,                                  \
			.name = _name,                                \
			.version = _version,                          \
			.init = _init,                                \
			.uninit = _uninit,                            \
			.set_parameter_value = _spv,                  \
			.get_paramter_value = _gpv                    \
		};                                                \
		return &_p;                                       \
	}                                                     \
	struct _unused_


#endif /* EVCPE_PLUGIN_H_ */
