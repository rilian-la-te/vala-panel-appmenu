project(
  'mate-hud-daemon',
  'c',
  version : '26.10-rc1',
  meson_version : '>=0.51.0',
  license : 'LGPL-3.0-or-later',
  default_options : ['c_std=gnu11', 'buildtype=debugoptimized', 'warning_level=2']
)

am_cflags = [
    '-fstack-protector',
    '-DGETTEXT_PACKAGE="vala-panel-appmenu"',
    '-DLOCALEDIR="@0@"'.format(join_paths(get_option('prefix'), get_option('localedir'))),
]
add_project_arguments(am_cflags, language: 'c')

prefix    = get_option('prefix')
datadir   = get_option('datadir')
bindir    = get_option('bindir')
localedir = get_option('localedir')
schema_dir = join_paths(datadir, 'glib-2.0', 'schemas')
autostart_dir = join_paths(get_option('sysconfdir'), 'xdg', 'autostart')
service_dir = join_paths(prefix, datadir, 'dbus-1', 'services')

gnome = import('gnome')
i18n = import('i18n')

gtk         = dependency('gtk+-3.0',    version: '>=3.22.0')
glib        = dependency('glib-2.0',   version: '>=2.52.0')
gio         = dependency('gio-2.0',    version: '>=2.52.0')
x11         = dependency('x11')
json_glib   = dependency('json-glib-1.0')

subdir('src')
subdir('data')
