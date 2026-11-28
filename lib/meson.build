sources = files(
    'backend.vala',
    'registrar.vala',
    'menu-provider.vala',
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

xcb_src = files(
    'appmenu-x11.vala',
    'xcb-aux.vapi',
)

xcb_c_src = files(
    'xcb-aux.c',
    'xcb-aux.h',
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

appmenu_deps = [giounix, importer_dep, posix_dep]
if backend_x11
    xcb_lib = static_library('xcb', xcb_c_src, dependencies: [xcb, xcb_icccm, xcb_ewmh, giounix])
    xcb_inline = declare_dependency(include_directories: include_directories('.'), link_whole: xcb_lib)
    sources += xcb_src
	appmenu_deps += xcb_inline
endif

if backend_wayfire
    sources += wayfire_src
    appmenu_deps += json_glib
endif

appmenu_lib = static_library('libmenuprovider',
        sources, config,
        dependencies: appmenu_deps,
        pic: true
    )
appmenu_inc = include_directories('.')

appmenu_dep = declare_dependency(
    include_directories: appmenu_inc,
    dependencies: [giounix],
    link_with: appmenu_lib
)
