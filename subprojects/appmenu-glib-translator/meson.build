project(
  'appmenu-glib-translator',
  'c',
version : '25.04',
meson_version : '>=0.61.0',
license : 'LGPL-3.0-or-later',
default_options : ['c_std=gnu11', 'buildtype=debugoptimized', 'warning_level=1']
)

gnome = import('gnome')
pkgconfig = import('pkgconfig')

glib_ver = '>=2.52.0'
giounix = dependency('gio-unix-2.0', version: glib_ver)
gdkpixbuf = dependency('gdk-pixbuf-2.0', required: false)

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

importer_name = 'appmenu-glib-translator'

importer_lib = library(importer_name, imp_sources, importer_enums_gen, imp_dbus,
    dependencies: [giounix, gdkpixbuf],
    version: meson.project_version(),
    install: true,
    soversion: 0,
    pic : true
)
importer_inc = include_directories('.')
imp_public = ['importer.h']
install_headers(imp_public, subdir : importer_name)

pkgconfig.generate(importer_lib,
             name: importer_name,
             description: 'A translator from DBusMenu to GMenuModel',
             requires: [giounix, gdkpixbuf],
             extra_cflags: ['-I${includedir}/'+importer_name]
            )

importer_gir = gnome.generate_gir(importer_lib,
                    sources: imp_public,
                    includes: ['GObject-2.0', 'Gio-2.0'],
                    header: imp_public,
                    namespace: 'AppmenuGLibTranslator',
                    identifier_prefix: 'DBusMenu',
                    symbol_prefix: 'dbus_menu',
                    nsversion: meson.project_version(),
                    install: true
)

importer_vapi = gnome.generate_vapi(importer_name,
    sources: importer_gir[0],
    packages: 'gio-2.0',
    install: true,
)

importer_dep = declare_dependency(
        include_directories: importer_inc,
        dependencies: [importer_vapi, giounix, gdkpixbuf],
        link_with: importer_lib
)
