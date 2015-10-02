/*
 * plugin.c
 *
 *  Created on: Sep 25, 2015
 *      Author: avalluri
 */

#include <dlfcn.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "config.h"
#include "log.h"
#include "tqueue.h"

#include "plugin.h"

#ifndef PLUGINS_DIR
#	define PLUGINS_DIR "/usr/lib/evcpe/plugins"
#endif

typedef struct {
	const char* name;
	evcpe_plugin_value_changed_cb_t listener;
	void* data;
} listener_info;

static
listener_info* _listener_info_new(const char* name,
		evcpe_plugin_value_changed_cb_t cb, void* data) {
	listener_info *info = (listener_info*)calloc(1, sizeof(*info));

	if (!info) return NULL;
	info->name = name;
	info->listener = cb;
	info->data = data;

	return info;
}

static
_listener_info_free(listener_info* info) {
	if (info) free(info);
}

static
int _listener_info_compare_func(listener_info* info, const char* name,
		void* data) {
	return info && name && strcmp(info->name, name);
}

struct _evcpe_plugin_priv {
	unsigned ref_count;
	void* handle;
	tqueue* value_change_listeners;
};


int evcpe_plugin_load(const char* name, evcpe_plugin** p_out) {
	void* handle = NULL;
	char  plugin_path[PATH_MAX];
	evcpe_plugin* (*loader)();
	int rc = 0;
	evcpe_plugin* p = NULL;

	if (!name || !p_out) return EINVAL;

	*p_out = NULL;

	snprintf(plugin_path, PATH_MAX, "%s/lib%s.so", PLUGINS_DIR, name);

	handle = dlopen(plugin_path, RTLD_LAZY|RTLD_GLOBAL);
	if (!handle) {
		ERROR("Failed to load plugin %s : %s", plugin_path, dlerror());
		return -1;
	}
	loader = dlsym(handle, "evcpe_plugin_get_object");
	if (!loader) {
		ERROR("Failed to load plugin object: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	if (!(p = loader()) || (rc = p->init(p))) {
		dlclose(handle);
		return -1;
		*p_out = NULL;
	}

	p->priv = calloc(1, sizeof(p->priv));
	p->priv->handle = handle;
	p->priv->ref_count = 1;
	p->priv->value_change_listeners = tqueue_new(
			(tqueue_compare_func_t)_listener_info_compare_func,
			(tqueue_free_func_t)_listener_info_free);

	*p_out = p;

	return rc;
}

void evcpe_plugin_unload(evcpe_plugin* p) {
	if (!p) return ;

	if (p->uninit) p->uninit(p);
	dlclose(p->priv->handle);
	if (p->priv->value_change_listeners) {
		tqueue_free(p->priv->value_change_listeners);
		p->priv->value_change_listeners = NULL;
	}
	free(p->priv);
}

evcpe_plugin* evcpe_plugin_ref(evcpe_plugin* p) {
	p && ++p->priv->ref_count;
	return p;
}

void evcpe_plugin_unref(evcpe_plugin* p) {
	if (p && ! --p->priv->ref_count) {
		evcpe_plugin_unload(p);
	}
}

void evcpe_plugin_set_value_change_listener(evcpe_plugin* p, const char* name,
		evcpe_plugin_value_changed_cb_t cb, void* data) {
	tqueue_element* node = NULL;
	listener_info *info = NULL;

	if (!p || !name || !cb) return;

	node = tqueue_find(p->priv->value_change_listeners, name);
	if (node) {
		info = node->data;
		info->listener = cb;
		info->data = data;
	}
	else {
		info = _listener_info_new(name, cb, data);
		tqueue_insert(p->priv->value_change_listeners, info);
	}
}

void evcpe_plugin_unset_value_change_listener(evcpe_plugin* p, const char* name)
{
	tqueue_element* node = NULL;

	if (!p || !name) return;

	node = tqueue_find(p->priv->value_change_listeners, name);
	if (node) tqueue_remove(p->priv->value_change_listeners, node);
}

void evcpe_plugin_emit_value_change(evcpe_plugin* p, const char* name,
		const char* value, unsigned value_len) {
	tqueue_element* node = NULL;
	listener_info* info = NULL;
	if (!p || !name) return;

	node = tqueue_find(p->priv->value_change_listeners, name);
	if (node && (info = node->data) && info->listener)
		info->listener(p, name, value, value_len, info->data);
}

