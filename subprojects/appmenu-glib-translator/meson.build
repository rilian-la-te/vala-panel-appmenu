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

common_sources = files(
    'definitions.h',
    'debug.c',
    'debug.h',
)
imp_sources = files(
    'importer/item.c',
    'importer/item.h',
    'importer/importer.c',
    'importer/importer.h',
    'importer/model.h',
    'importer/model.c',
    'importer/section.c',
    'importer/section.h',
    'importer/utils.c',
    'importer/utils.h'
    )
exp_sources = files(
    'exporter/gtkactionmuxer.c',
    'exporter/gtkactionmuxer.h',
    'exporter/gtkactionobservable.c',
    'exporter/gtkactionobservable.h',
    'exporter/gtkactionobserver.c',
    'exporter/gtkactionobserver.h',
    'exporter/gtkmenutracker.c',
    'exporter/gtkmenutracker.h',
    'exporter/gtkmenutrackeritem.c',
    'exporter/gtkmenutrackeritem.h',
)
common_headers = files('definitions.h')
enum = 'translator-enums'
translator_enums_gen = gnome.mkenums(
  enum,
  sources: common_headers,
  c_template: enum + '.c.template',
  h_template: enum + '.h.template',
)
translator_dbus = gnome.gdbus_codegen(
    'dbusmenu-interface',
    sources: 'com.canonical.dbusmenu.xml',
    interface_prefix: 'com.canonical',
    autocleanup: 'all',
    namespace: 'DBusMenu'
)
translator_marshall = gnome.genmarshal('exporter-marshall',
    sources: 'exporter/marshall.list',
    valist_marshallers: true,
    internal: true
)

translator_name = 'appmenu-glib-translator'

translator_inc = include_directories('.', 'importer')

translator_lib = library(translator_name, 
common_sources, translator_enums_gen, translator_dbus, imp_sources, exp_sources, translator_marshall,
    dependencies: [giounix, gdkpixbuf],
    include_directories: translator_inc,
    version: meson.project_version(),
    install: true,
    soversion: 0,
    pic : true
)
translator_public = ['importer/importer.h']
install_headers(translator_public, subdir : translator_name)

pkgconfig.generate(translator_lib,
             name: translator_name,
             description: 'A translator from DBusMenu to GMenuModel',
             requires: [giounix],
             requires_private: [gdkpixbuf],
             extra_cflags: ['-I${includedir}/'+translator_name]
            )

translator_gir = gnome.generate_gir(translator_lib,
                    sources: translator_public,
                    includes: ['GObject-2.0', 'Gio-2.0'],
                    header: translator_public,
                    namespace: 'AppmenuGLibTranslator',
                    identifier_prefix: 'DBusMenu',
                    symbol_prefix: 'dbus_menu',
                    nsversion: meson.project_version(),
                    install: true
)

translator_vapi = gnome.generate_vapi(translator_name,
    sources: translator_gir[0],
    packages: 'gio-2.0',
    install: true,
)

translator_dep = declare_dependency(
        include_directories: translator_inc,
        dependencies: [translator_vapi, giounix, gdkpixbuf],
        link_with: translator_lib
)
