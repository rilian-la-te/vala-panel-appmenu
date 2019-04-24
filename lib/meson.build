sources = files(
    'appmenu-abstractions.vala',
    'appmenu-bamf.vala',
    'registrar.vala',
    'menu-widget.vala',
    'helper-desktop.vala',
    'helper-dbus.vala',
    'helper-dbusmenu.vala',
    'helper-menumodel.vala',
    'launcher.c',
    'launcher.h'
)

matcher_src = files(
    'matcher.c',
    'matcher.h',
    'matcher.vapi'
)

res = gnome.compile_resources(
    'lib', 'libappmenu.gresource.xml',
    source_dir: '.',
    c_name: 'lib'
)


