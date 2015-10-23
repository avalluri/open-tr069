/*
 * plugin.c
 *
 *  Created on: Sep 25, 2015
 *      Author: avalluri
 */

#include "plugin-priv.h"

void evcpe_plugin_emit_value_change(evcpe_plugin* p, const char* name,
		const char* value, unsigned value_len) {
	tqueue_element* node = NULL;
	listener_info* info = NULL;
	if (!p || !name) return;

	node = tqueue_find(p->priv->value_change_listeners, name);
	if (node && (info = node->data) && info->listener)
		info->listener(p, name, value, value_len, info->data);
}

