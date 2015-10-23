#ifndef _EVCPE_PLUGIN_PRIV_H
#define _EVCPE_PLUGIN_PRIV_H

#include "plugin.h"
#include "tqueue.h"

typedef void (*evcpe_plugin_value_changed_cb_t)(evcpe_plugin* p,
		const char* name, const char* value, unsigned value_len, void* data);

typedef struct {
        const char* name;
        evcpe_plugin_value_changed_cb_t listener;
        void* data;
} listener_info;

struct _evcpe_plugin_priv {
        unsigned ref_count;
        void* handle;
        tqueue* value_change_listeners;
};

int evcpe_plugin_load(const char* name, evcpe_plugin** plugin_out);
void evcpe_plugin_unload(evcpe_plugin* self);
evcpe_plugin* evcpe_plugin_ref(evcpe_plugin* self);
void evcpe_plugin_unref(evcpe_plugin* self);

void evcpe_plugin_set_value_change_listener(evcpe_plugin* p, const char* name,
		evcpe_plugin_value_changed_cb_t cb, void* data);
void evcpe_plugin_unset_value_change_listener(evcpe_plugin* p, const char* name);


#endif // _EVCPE_PLUGIN_PRIV_H
