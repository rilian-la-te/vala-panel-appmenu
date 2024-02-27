install_data([
    'org.appmenu.gtk-module.gschema.xml'
], install_dir: schema_dir)

systemd = dependency('systemd', required: false)

if systemd.found()
    userunitdir = systemd.get_variable(pkgconfig:'systemduserunitdir')
    configure_file(
        input: 'appmenu-gtk-module.service.in',
        output: 'appmenu-gtk-module.service',
        install_dir: userunitdir,
        configuration: {'CMAKE_INSTALL_FULL_BINDIR': join_paths(prefix,get_option('bindir'))}
    )
endif
