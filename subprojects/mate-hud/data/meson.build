install_data('mate-hud-daemon.desktop',
    install_dir : autostart_dir,
)

configure_file(
    input : 'org.mate.HUD.service.in',
    output : 'org.mate.HUD.service',
    install_dir : service_dir,
    configuration : {
        'BINDIR' : join_paths(prefix, bindir),
    },
)
