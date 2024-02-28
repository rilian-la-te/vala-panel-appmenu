lib_sources = files(
	'unity-gtk-action-group.c',
	'unity-gtk-action.c',
	'unity-gtk-menu-shell.c',
	'unity-gtk-menu-section.c',
	'unity-gtk-menu-item.c',
)
lib_private_headers = [
	'unity-gtk-action-group-private.h',
    'unity-gtk-action-private.h',
    'unity-gtk-menu-shell-private.h',
    'unity-gtk-menu-section-private.h',
    'unity-gtk-menu-item-private.h',
]
lib_headers = files(
	'appmenu-gtk-action-group.h',
	'appmenu-gtk-menu-shell.h',
	'appmenu-gtk-parser.h'
)

parser_inc = include_directories('.')
parser_kwargs = {
    'sources': lib_sources+lib_headers+lib_private_headers,
    'install': true,
    'version': meson.project_version(),
    'soversion': '0'
}
pc_kwargs = {
    'description': 'GtkMenuShell to GMenuModel parser',
    'variables': ['exec_prefix=@0@'.format(join_paths(prefix,get_option('libexecdir')))],
    'extra_cflags': ['-I${includedir}/appmenu-gtk-parser']
}

if build_gtk2
    gtk2_parser_lib = shared_library(
        'appmenu-gtk2-parser',
        dependencies: gtk2,
        c_args: '-Wno-deprecated-declarations',
        kwargs: parser_kwargs
    )
    pkgconfig.generate(gtk2_parser_lib,
        requires: gtk2,
        kwargs: pc_kwargs
    )
    gtk2_parser_dep = declare_dependency(
        include_directories: parser_inc,
        link_with: gtk2_parser_lib,
        dependencies: gtk2
    )
endif

if build_gtk3
    gtk3_parser_lib = shared_library(
        'appmenu-gtk3-parser',
        dependencies: gtk3,
        kwargs: parser_kwargs
    )
    pkgconfig.generate(gtk3_parser_lib,
        requires: gtk3,
        kwargs: pc_kwargs
    )
    gtk3_parser_dep = declare_dependency(
        include_directories: parser_inc,
        link_with: gtk3_parser_lib,
        dependencies: gtk3
    )
endif

if build_gtk2 or build_gtk3
    install_headers(lib_headers, subdir: 'appmenu-gtk-parser')
endif
