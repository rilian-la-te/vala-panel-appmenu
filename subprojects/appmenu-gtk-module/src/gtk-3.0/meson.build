gtk3_module = shared_module(
    'appmenu-gtk-module', module_sources,
    dependencies: gtk3_parser_dep,
    install: true,
    install_dir: join_paths(gtk3.get_variable(pkgconfig:'libdir'),'gtk-3.0','modules')
)
