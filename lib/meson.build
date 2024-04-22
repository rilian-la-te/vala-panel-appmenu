sources = files(
    'appmenu-abstractions.vala',
    'registrar.vala',
    'menu-widget.vala',
    'helper-desktop.vala',
    'helper-dbus.vala',
    'helper-dbusmenu.vala',
    'helper-menumodel.vala',
    'launcher.vapi',
    'launcher.c',
    'launcher.h'
)

wnck_src = files(
    'appmenu-wnck.vala',
    'matcher.c',
    'matcher.h',
    'libwnck-aux.c',
    'libwnck-aux.h',
    'matcher.vapi'
)

libres = gnome.compile_resources(
    'lib', 'libappmenu.gresource.xml',
    source_dir: '.',
    c_name: 'lib'
)
valac = meson.get_compiler('vala')
posix_dep = valac.find_library('posix')

importer_dep = dependency('appmenu-glib-translator', fallback : ['appmenu-glib-translator', 'importer_dep'])

appmenu_deps = [giounix, gtk, importer_dep, posix_dep]
appmenu_cflags = []
if backend_wnck
    sources += wnck_src
    appmenu_deps += wnck
    appmenu_cflags += ['-DWNCK_I_KNOW_THIS_IS_UNSTABLE']
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
