/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "consts.h"
#include "settings.h"

static GKeyFile *cached_settings = NULL;

/* Merge config files, so, last one wins */
static void merge_keyfile(GKeyFile *dst, GKeyFile *src)
{
	gsize   n_groups = 0;
	char  **groups   = g_key_file_get_groups(src, &n_groups);
	for (gsize i = 0; i < n_groups; i++) {
		gsize n_keys = 0;
		char **keys  = g_key_file_get_keys(src, groups[i], &n_keys, NULL);
		for (gsize j = 0; j < n_keys; j++) {
			GError *error = NULL;
			char   *value = g_key_file_get_value(src, groups[i], keys[j], &error);
			if (value != NULL) {
				g_key_file_set_value(dst, groups[i], keys[j], value);
				g_free(value);
			} else if (error != NULL) {
				g_clear_error(&error);
			}
		}
		g_strfreev(keys);
	}
	g_strfreev(groups);
}

static void merge_config_file(GKeyFile *key_file, const char *path)
{
	g_autoptr(GKeyFile) file = g_key_file_new();
	if (g_key_file_load_from_file(file, path, G_KEY_FILE_KEEP_COMMENTS, NULL))
		merge_keyfile(key_file, file);
}

/* Handle XDG_CURRENT_DESKTOP to allow per-desktop settings*/
static void load_desktop_config(GKeyFile *key_file, const char *dir, const char *desktop)
{
	g_autofree char *lower = g_ascii_strdown(desktop, -1);
	g_autofree char *name  = g_strdup_printf("%s-%s", lower, APPMENU_CONFIG_FILE);
	g_autofree char *path  = g_build_filename(dir, name, NULL);
	merge_config_file(key_file, path);
}

static void load_desktop_config_for_dir(GKeyFile *key_file, const char *dir)
{
	const char *current = g_getenv("XDG_CURRENT_DESKTOP");
	if (current == NULL || *current == '\0')
		return;
	g_auto(GStrv) desktops = g_strsplit(current, ":", 2);
	if (desktops[0] == NULL || *desktops[0] == '\0')
		return;
	if (desktops[1] != NULL && *desktops[1] != '\0')
		load_desktop_config(key_file, dir, desktops[1]);

	load_desktop_config(key_file, dir, desktops[0]);
}

static void load_generic_config(GKeyFile *key_file, const char *dir)
{
	g_autofree char *path = g_build_filename(dir, APPMENU_CONFIG_FILE, NULL);
	merge_config_file(key_file, path);
}

static GKeyFile *load_settings(void)
{
	GKeyFile *key_file = g_key_file_new();

	for (const char *const *dir = g_get_system_config_dirs(); *dir != NULL; dir++)
		load_generic_config(key_file, *dir);
	for (const char *const *dir = g_get_system_config_dirs(); *dir != NULL; dir++)
		load_desktop_config_for_dir(key_file, *dir);

	load_generic_config(key_file, g_get_user_config_dir());
	load_desktop_config_for_dir(key_file, g_get_user_config_dir());

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
