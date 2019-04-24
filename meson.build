project('vala-panel-appmenu', 'vala', 'c',
	version : '0.7.1',
	meson_version: '>=0.49.0',
	license: ['LGPL-3.0'],
    default_options: [
        'c_std=gnu11',
        'buildtype=debugoptimized',
        'warning_level=1'
        ])

am_cflags = [
    '-fstack-protector',
    '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
]
add_project_arguments(am_cflags, language: 'c')

##################
# Module imports #
##################

gnome = import('gnome')
i18n = import('i18n')

###############
# Directories #
###############
prefix = get_option('prefix')
datadir = get_option('datadir')
localedir = get_option('localedir')
schema_dir = join_paths(get_option('datadir'), 'glib-2.0', 'schemas')


################
# Dependencies #
################
gtk_ver = '>=3.22.0'
gtk = dependency('gtk+-3.0', version: gtk_ver)

bamf_ver = '>=0.5.0'
bamf = dependency('libbamf3', version: bamf_ver)

vp_ver = '>=0.4.88'
vp = dependency('vala-panel', version:  vp_ver, required: get_option('valapanel'))
vala_panel_found = vp.found()

mp_ver = '>=1.20'
mp = dependency('libmatepanelapplet-4.0', version: mp_ver, required: get_option('mate'))
mate_found = mp.found()

xp = dependency('libxfce4panel-2.0', required: get_option('xfce'))
xc = dependency('libxfconf-0', required: get_option('xfce'))
xfce_found = xp.found() and xc.found()

bp = dependency('budgie-1.0', required: get_option('budgie'))
budgie_found = bp.found()


#################
# Configuration #
#################
conf_data = configuration_data()
conf_data.set('GETTEXT_PACKAGE', meson.project_name())
conf_data.set('LOCALE_DIR', join_paths(prefix,localedir))
conf_data.set('CMAKE_INSTALL_PREFIX', prefix)

config = configure_file(input : join_paths('vapi','config.vala.in'),
               output : 'config.vala',
			   configuration : conf_data)
			  
desktop_kwargs = {
      'type': 'desktop',
      'args' : ['--keyword=Description'],
      'po_dir': join_paths(meson.current_source_dir(),'po'),
      'install': true,
}
			   

subdir('lib')
subdir('data')
subdir('po')

readmes = [
    'README.md',
    'LICENSE', 
]
install_data(readmes, install_dir : join_paths(get_option('datadir'), meson.project_name(), 'doc'))
