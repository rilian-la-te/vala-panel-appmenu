pkgconfig = import('pkgconfig')

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
imp_headers = files('definitions.h', 'importer.h')
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

importer_name = 'menutranslator'

importer_lib = library(importer_name, imp_sources, importer_enums_gen, imp_dbus,
    dependencies: [giounix, gdkpixbuf],
    version: meson.project_version(),
    install: true,
    soversion: 0,
    pic : true
)
importer_inc = include_directories('.')
importer_dep = declare_dependency(
        include_directories: importer_inc,
        link_with: importer_lib
)
install_headers(imp_headers, subdir : importer_name)
pkgconfig.generate(importer_lib,
             name: importer_name,
             description: 'A translator from DBusMenu to GMenuModel',
             requires: [giounix, gdkpixbuf],
             extra_cflags: ['-I${includedir}/menutranslator']
            )
