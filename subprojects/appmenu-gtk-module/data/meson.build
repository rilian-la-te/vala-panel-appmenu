sysconf_xdg_dir = join_paths(get_option('sysconfdir'), 'xdg')

install_data('appmenu-gtk-module.conf',
    install_dir: sysconf_xdg_dir
)

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
