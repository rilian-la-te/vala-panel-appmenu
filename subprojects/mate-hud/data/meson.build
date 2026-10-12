i18n.merge_file(
    input : 'mate-hud-daemon.desktop.in',
    output : 'mate-hud-daemon.desktop',
    type : 'desktop',
    po_dir : join_paths(meson.current_source_dir(), '..', '..', '..', 'po'),
    install : true,
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
