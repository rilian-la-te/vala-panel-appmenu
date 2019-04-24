imp_sources = files(
    'definitions.h',
    'debug.c',
    'debug.h',
    'item.c',
    'item.h',
    'importer.c',
    'importer.h',
    'model.h',
    'model.c',
    'section.c',
    'section.h',
    'utils.c',
    'utils.h'
    )
imp_headers = files('definitions.h')
enum = 'importer-enums'
importer_enums_gen = gnome.mkenums(
  enum,
  sources: imp_headers,
  c_template: enum + '.c.template',
  h_template: enum + '.h.template',
)
imp_dbus = gnome.gdbus_codegen(
    'dbusmenu-interface',
    sources: 'com.canonical.dbusmenu.xml',
    interface_prefix: 'com.canonical',
    autocleanup: 'all',
    namespace: 'DBusMenu'
)

importer_lib = static_library('importer',imp_sources, importer_enums_gen, imp_dbus,
    dependencies: [giounix, gdkpixbuf],
    pic : true
)
importer_inc = include_directories('.')
importer_dep = declare_dependency(
        include_directories: importer_inc,
        link_whole: importer_lib
)
