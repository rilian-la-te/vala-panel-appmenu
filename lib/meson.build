sources = files(
    'backend.vala',
    'registrar.vala',
    'menu-provider.vala',
    'menu-widget.vala',
    'helper.vala',
    'helper-desktop.vala',
    'helper-dbus.vala',
    'helper-dbusmenu.vala',
    'helper-menumodel.vala',
    'launcher.vapi',
    'launcher.c',
    'launcher.h',
    'register.c',
    'matcher.c',
    'matcher.h',
    'matcher.vapi',
)

wnck_src = files(
    'appmenu-wnck.vala',
    'libwnck-aux.c',
    'libwnck-aux.vapi',
    'libwnck-aux.h',
)

wayfire_src = files(
    'appmenu-wayfire.vala'
) 

libres = gnome.compile_resources(
    'lib', 'ui/libappmenu.gresource.xml',
    source_dir: 'ui',
    c_name: 'lib'
)
valac = meson.get_compiler('vala')
posix_dep = valac.find_library('posix')

importer_dep = dependency('appmenu-glib-translator', version: '>=26.12', fallback : ['appmenu-glib-translator', 'translator_dep'])

appmenu_deps = [giounix, gtk, importer_dep, posix_dep]
appmenu_cflags = []
if backend_wnck
    gdkx_dep = valac.find_library('gdk-x11-3.0')
    sources += wnck_src
    appmenu_deps += [wnck, gdkx_dep]
    appmenu_cflags += ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE']
endif

if backend_wayfire
    sources += wayfire_src
    appmenu_deps += json_glib
endif

appmenu_lib = static_library('libappmenu',
        sources, config,
        dependencies: appmenu_deps,
        c_args: appmenu_cflags,
        pic: true
    )
appmenu_inc = include_directories('.')

appmenu_dep = declare_dependency(
    include_directories: appmenu_inc,
    dependencies: [gtk, giounix],
    link_whole: appmenu_lib
)
