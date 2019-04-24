subdir('dbusmenu-importer')
sources = files(
    'appmenu-abstractions.vala',
    'appmenu-bamf.vala',
    'registrar.vala',
    'menu-widget.vala',
    'helper-desktop.vala',
    'helper-dbus.vala',
    'helper-dbusmenu.vala',
    'helper-menumodel.vala',
    'launcher.vapi',
    'dbus-menu-importer.vapi',
    'launcher.c',
    'launcher.h'
)

matcher_src = files(
    'matcher.c',
    'matcher.h',
    'matcher.vapi'
)

libres = gnome.compile_resources(
    'lib', 'libappmenu.gresource.xml',
    source_dir: '.',
    c_name: 'lib'
)
valac = meson.get_compiler('vala')
posix_dep = valac.find_library('posix')
vver = valac.version()
addons_versions = {
    '<0.37.0': '<0.36.18',
    '<0.41.0': '<0.40.14',
    '<0.43.0': '<0.42.6',
    '<0.45.0': '<0.44.0'
}
foreach series, ver : addons_versions
    if vver.version_compare(series) and vver.version_compare(ver)
        addons_needed = true
    endif
endforeach
if addons_needed
    sources += files('../vapi/gio-addons-2.0.vapi')
endif
appmenu_deps = [giounix, gtk, bamf, importer_dep, posix_dep]
appmenu_cflags = []
if wnck.found()
    sources += matcher_src
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
