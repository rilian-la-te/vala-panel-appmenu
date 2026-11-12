/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <stdbool.h>
#include <string.h>
#include <wayland-client.h>

struct probe_ctx
{
	const char *want;
	bool found;
};

static void probe_global(void *data, struct wl_registry *reg, uint32_t name, const char *iface,
                         uint32_t version)
{
	(void)reg;
	(void)name;
	(void)version;
	struct probe_ctx *c = data;
	if (strcmp(iface, c->want) == 0)
		c->found = true;
}

static void probe_remove(void *data, struct wl_registry *reg, uint32_t name)
{
	(void)data;
	(void)reg;
	(void)name;
}

static const struct wl_registry_listener probe_listener = {
	.global        = probe_global,
	.global_remove = probe_remove,
};

bool wl_has(const char *interface)
{
	bool result          = false;
	struct wl_display *d = wl_display_connect(NULL);
	if (!d)
		return false;

	struct wl_registry *r = wl_display_get_registry(d);
	if (r)
	{
		struct probe_ctx ctx = { .want = interface, .found = false };
		wl_registry_add_listener(r, &probe_listener, &ctx);
		if (wl_display_roundtrip(d) >= 0)
			result = ctx.found;
		wl_registry_destroy(r);
	}

	wl_display_disconnect(d);
	return result;
}