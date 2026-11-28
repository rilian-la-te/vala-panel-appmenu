sources = files(
    'menu-widget.vala',
)

libres = gnome.compile_resources(
    'lib', 'libmenuwidget.gresource.xml',
    source_dir: '.',
    c_name: 'widget'
)
valac = meson.get_compiler('vala')
posix_dep = valac.find_library('posix')

widget_lib = static_library('libmenuwidget-gtk3',
    sources,
    dependencies: [gtk, appmenu_dep],
    pic: true
)
widget_inc = include_directories('.')
widget_dep = declare_dependency(
    include_directories: widget_inc,
    dependencies: [gtk, giounix],
    link_whole: widget_lib
)