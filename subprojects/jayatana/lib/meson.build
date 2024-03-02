install_directory = join_paths(get_option('libdir') / 'jayatana')

config = configure_file(input: 'config.h.in', output: 'config.h', configuration : data)
agent_sources = files(
  'com_jarego_jayatana_Agent.h',
  'com_jarego_jayatana_Agent.c'
)
jayatanaag = shared_library('jayatanaag', agent_sources, config,
          dependencies : [x11, jni],
          install: true,
          install_dir: install_directory
)

jayatana_sources = files(
  'com_jarego_jayatana_basic_GlobalMenu.c',
  'com_jarego_jayatana_basic_GMainLoop.c',
  'com_jarego_jayatana_jkey2xkey.c',
  'com_jarego_jayatana_jkey2xkey.h',
  'com_jarego_jayatana_jni.c',
  'com_jarego_jayatana_jni.h',
  'jayatana-window.c',
  'jayatana-window.h',
  'dbusmenu-definitions.h'
)

jayatana = shared_library('jayatana', jayatana_sources,
            dependencies: [x11, jni, xkbcommon, dbusmenu, jayatana_jni, glib],
            install: true,
            install_dir: install_directory
)
