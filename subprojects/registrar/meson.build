project('registrar', 'c',
	meson_version: '>=0.49.0',
    version: '0.7',
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
libexecdir = get_option('libexecdir')
installdir = join_paths(libexecdir, 'vala-panel')
servicedir = join_paths(prefix,datadir, 'dbus-1','services')

################
# Dependencies #
################
glib_ver = '>=2.52.0'
giounix = dependency('gio-unix-2.0', version: glib_ver)


#################
# Configuration #
#################
conf_data = configuration_data()
conf_data.set('GETTEXT_PACKAGE', 'vala-panel-appmenu')
conf_data.set('CMAKE_INSTALL_FULL_LOCALEDIR', join_paths(prefix,localedir))

config = configure_file(input : 'config.h.in',
               output : 'config.h',
			   configuration : conf_data)

version = vcs_tag(
	command : ['git','describe','--tags'],
	input: 'version.h.in',
	output: 'version.h',
	replace_string: '@VERSION@',
)

#################
#     Build     #
#################

marshal = gnome.genmarshal('registrar-marshal', sources: 'registrar-marshal.list')
intro_c = run_command(
        'cat', join_paths('data','com.canonical.AppMenu.Registrar.xml'),
    ).stdout().strip().split('"')
priv_c = run_command(
        'cat', join_paths('data','org.valapanel.AppMenu.Registrar.xml'),
    ).stdout().strip().split('"')

intro_xml = ''.join('\"'.join(intro_c).split('\n'))
priv_xml = ''.join('\"'.join(priv_c).split('\n'))

xml = configure_file(input : 'registrar-xml.c.in',
               output : 'registrar-xml.c',
			   configuration : {
                    'XML_CONTENTS' : intro_xml,
                    'PRIVATE_CONTENTS' : priv_xml
			   })
sources = files(
    'registrar-main.c',
    'registrar-main.h',
    'registrar-dbusmenu.c',
    'registrar-dbusmenu.h'
)
registrar = executable('appmenu-registrar',
    config, xml, sources, marshal, version,
    dependencies : giounix,
    install : true,
    install_dir : installdir
)

service = configure_file(input : join_paths('data', 'appmenu-registrar.service.in'),
               output : 'com.canonical.AppMenu.Registrar.service',
               install_dir: servicedir,
			   configuration : {
                    'CMAKE_INSTALL_FULL_LIBEXECDIR' : join_paths(prefix,libexecdir),
			   })
