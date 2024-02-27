project('appmenu-gtk-module', 'c',
	meson_version: '>=0.61.0',
	version: '24.02',
	license: ['LGPL-3.0-or-later'],
    default_options: [
        'c_std=gnu11',
        'buildtype=debugoptimized',
        'warning_level=1'
        ])

##################
# Module imports #
##################

gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

###############
# Directories #
###############
prefix = get_option('prefix')
datadir = get_option('datadir')
localedir = get_option('localedir')
schema_dir = join_paths(get_option('datadir'), 'glib-2.0', 'schemas')
docdir = join_paths(prefix,datadir, 'doc')

################
# Dependencies #
################
glib_ver = '>=2.52.0'
giounix = dependency('gio-unix-2.0', version: glib_ver)

gtk_versions = get_option('gtk')
gtk2_requested = '2' in gtk_versions
gtk3_requested = '3' in gtk_versions

gtk2_ver = '>=2.24.0'
gtk3_ver = '>=3.22.0'

gtk2 = dependency('gtk+-2.0', version: gtk2_ver, required: gtk2_requested)
gtk3 = dependency('gtk+-3.0', version: gtk3_ver, required: gtk3_requested)

build_gtk2 = gtk2.found()
build_gtk3 = gtk3.found()

#################
# Configuration #
#################

readmes = [
    'README.md',
    'TODO',
    'AUTHORS'
]
install_data(readmes, install_dir : join_paths(get_option('datadir'), 'doc', meson.project_name()))
install_data('LICENSE', install_dir : join_paths(get_option('datadir'), 'licenses', meson.project_name()))


subdir('lib')
subdir('src')
subdir('data')
subdir('docs')
subdir('tests')
