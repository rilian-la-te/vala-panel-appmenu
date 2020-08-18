ent_dict = {
    'CMAKE_PROJECT_NAME' : meson.project_name(),
    'PACKAGE_BUGREPORT': 'https://github.com/rilian-la-te/vala-panel-appmenu/issues',
    'PACKAGE_STRING' : 'appmenu-gtk-parser',
    'CPACK_SOURCE_PACKAGE_FILE_NAME': '@0@-@1@.tar.xz'.format(meson.project_name(),meson.project_version()),
    'PACKAGE_URL' : 'https://github.com/rilian-la-te/vala-panel-appmenu',
    'VERSION': meson.project_version(),
    'PACKAGE_API_VERSION': meson.project_version()
}
ent = configure_file(
    input: 'gtkdocentities.ent.in',
    output: 'gtkdocentities.ent',
    configuration: ent_dict
)

if get_option('gtk_doc')
gnome.gtkdoc(
    'appmenu-gtk-parser',
    content_files: ent,
    src_dir: parser_inc,
    gobject_typesfile: 'appmenu-gtk-parser.types',
    dependencies: gtk3_parser_dep,
    ignore_headers: lib_private_headers,
    install: true,
    main_sgml: join_paths(meson.current_build_dir(),'appmenu-gtk-parser-docs.sgml')
)

endif
