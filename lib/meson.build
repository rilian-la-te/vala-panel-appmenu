subdir('dbusmenu-importer')
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
vver = valac.version()
addons_versions = {
    '>0.35.0': '<0.36.18',
    '>0.39.0': '<0.40.14',
    '>0.41.0': '<0.42.6',
    '>0.43.0': '<0.44.0'
}
foreach series, ver : addons_versions
    if vver.version_compare(series) and vver.version_compare(ver)
            sources += files('../vapi/gio-addons-2.0.vapi')
        break
    endif
endforeach

appmenu_deps = [giounix, gtk, importer_dep, posix_dep, importer_vapi]
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
