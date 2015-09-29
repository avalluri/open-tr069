/*
 * evcpe_plugin.h
 *
 *  Created on: Sep 24, 2015
 *      Author: avalluri
 */

#ifndef EVCPE_PLUGIN_H_
#define EVCPE_PLUGIN_H_

typedef struct _evcpe_plugin evcpe_plugin;
typedef struct _evcpe_plugin_priv evcpe_plugin_priv;

typedef void (*evcpe_plugin_value_changed_cb_t)(evcpe_plugin* p,
		const char* name, const char* value, unsigned value_len, void* data);

struct _evcpe_plugin
{
	const char *name;
	const char *version;
	evcpe_plugin_priv* priv;

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

void evcpe_plugin_set_value_change_listener(evcpe_plugin* p, const char* name,
		evcpe_plugin_value_changed_cb_t cb, void* data);
void evcpe_plugin_unset_value_change_listener(evcpe_plugin* p, const char* name);

/* API for Plugins */
void evcpe_plugin_emit_value_change(evcpe_plugin* p, const char* name,
		const char* value, unsigned value_len);

#define EVCPE_PLUGIN_REGISTER(\
                      _name, _version, _init, _spv, _gpv) \
	evcpe_plugin* evcpe_plugin_get_object() {   \
		static evcpe_plugin _p = {              \
			.name = _name,                      \
			.version = _version,                \
			.init = _init,                      \
			.uninit = _uninit,                  \
			.set_parameter_value = _spv,        \
			.get_paramter_value = _gpv          \
		};                                      \
		return &_p;                             \
	}                                           \
	struct _unused_


#endif /* EVCPE_PLUGIN_H_ */
