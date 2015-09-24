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

#include "config.h"
#include "log.h"

#include "plugin.h"

#ifndef PLUGINS_DIR
#	define PLUGINS_DIR "/usr/lib/evcpe/plugins"
#endif

int evcpe_plugin_load(const char* name, evcpe_plugin** p) {
	void* handle = NULL;
	char  plugin_path[PATH_MAX];
	evcpe_plugin* (*loader)(void* h);
	int rc = 0;

	if (!name || !p) return EINVAL;

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

	*p = loader(handle);

	if ((rc = (*p)->init(*p))) {
		dlclose(handle);
		*p = NULL;
	}

	return rc;
}

void evcpe_plugin_unload(evcpe_plugin* p) {
	if (!p) return ;

	if (p->uninit) p->uninit(p);
	dlclose(p->handle);
}

evcpe_plugin* evcpe_plugin_ref(evcpe_plugin* p) {
	p && ++p->ref_count;
	return p;
}

void evcpe_plugin_unref(evcpe_plugin* p) {
	if (p) {
		if (! --p->ref_count) {
			evcpe_plugin_unload(p);
		}
	}
}
