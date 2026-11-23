/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "consts.h"
#include "settings.h"

static GKeyFile *cached_settings = NULL;

static void load_config_file(GKeyFile *key_file, const char *dir)
{
	char *path = g_build_filename(dir, APPMENU_CONFIG_FILE, NULL);
	g_key_file_load_from_file(key_file, path, G_KEY_FILE_KEEP_COMMENTS, NULL);
	g_free(path);
}

static GKeyFile *load_settings(void)
{
	GKeyFile *key_file = g_key_file_new();

	for (const char *const *dir = g_get_system_config_dirs(); *dir != NULL; dir++)
		load_config_file(key_file, *dir);

	load_config_file(key_file, g_get_user_config_dir());

	return key_file;
}

G_GNUC_INTERNAL GKeyFile *appmenu_settings_get(void)
{
	if (g_atomic_pointer_get(&cached_settings) != NULL)
		return cached_settings;

	static GMutex init_mutex;
	g_mutex_lock(&init_mutex);

	if (cached_settings == NULL) {
		GKeyFile *fresh = load_settings();
		g_atomic_pointer_set(&cached_settings, fresh);
	}

	g_mutex_unlock(&init_mutex);
	return cached_settings;
}
