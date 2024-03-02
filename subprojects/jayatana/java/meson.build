subdir('com/jarego/jayatana')

jayatana_java_sources = files(
  'com/jarego/jayatana/Feature.java',
  'com/jarego/jayatana/FeatureManager.java',
  'com/jarego/jayatana/FeatureWrapper.java',
  'com/jarego/jayatana/basic/GMainLoop.java',
  'com/jarego/jayatana/basic/GlobalMenuAdapter.java',
  'com/jarego/jayatana/basic/GlobalMenu.java',
  'com/jarego/jayatana/swing/SwingGlobalMenu.java',
  'com/jarego/jayatana/swing/SwingGTKFixed.java',
  'com/jarego/jayatana/swing/SwingWMClass.java',
  'com/jarego/jayatana/swing/SwingGlobalMenuWindow.java'
)
jayatana_java_headers = files(
  'com/jarego/jayatana/basic/GMainLoop.java',
  'com/jarego/jayatana/basic/GlobalMenu.java',
  'com/jarego/jayatana/Feature.java'
)
args = ['--add-exports', 'java.desktop/com.sun.java.swing.plaf.gtk=ALL-UNNAMED']
java_install_path = join_paths(get_option('datadir'), 'java')

jayatana_java = jar('jayatana', jayatana_java_sources, native_libraries,
                        java_args : args,
                        install: true,
                        install_dir: java_install_path
                       )

jayatana_jni_hdr = java.native_headers(
    jayatana_java_headers,
    package: 'com.jarego.jayatana',
    classes: ['GMainLoop', 'GlobalMenu']
)

jayatana_jni = declare_dependency(
            sources: jayatana_jni_hdr
)

manifest = configure_file(
                input: 'MANIFEST.MF.in',
                output: 'MANIFEST.MF',
                configuration: data)


agent_java = jar('jayatanaag', agent,
                        java_args : args,
                        install: true,
                        install_dir: java_install_path
)

# HACK: regenerate manifest after building a JAR file
agent_manifest_patch = custom_target(
                                  output: 'jayatanaag.timestamp',
                                  command: ['jar', '-f', agent_java.full_path(), '-u', '-m', manifest, '-v'],
                                  capture: true,
                                  depends: agent_java,
                                  build_by_default : true,
                                  build_always_stale: true
)