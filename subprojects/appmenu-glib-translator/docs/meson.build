gidocgen = find_program('gi-docgen', required: get_option('docs'))

docs_dir = get_option('datadir') / 'doc'

custom_target(
  'appmenu-glib-translator-doc',
  input: [
    'docs.toml',
    translator_gir[0],
  ],
  output: 'AppmenuGLibTranslator-@0@'.format(meson.project_version()),
  command: [
    gidocgen,
    'generate',
    '--quiet',
    '--config=@INPUT0@',
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT1@',
  ],
  depend_files: [
    'docs.toml',
  ],
  build_by_default: true,
  install: true,
  install_dir: docs_dir,
)