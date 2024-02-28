gtk2_module = shared_module(
    'appmenu-gtk-module', module_sources,
    dependencies: gtk2_parser_dep,
    c_args: '-Wno-deprecated-declarations',
    install: true,
    install_dir: join_paths(gtk2.get_variable(pkgconfig:'libdir'),'gtk-2.0','modules')
)
