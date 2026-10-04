daemon_sources = files(
    'main.c',
    'shortcut.c',
    'activate.c',
    'window.c',
    'model.c',
    'collect.c',
    'collect-dbusmenu.c',
    'collect-menumodel.c',
    'history.c',
    'mnemonic.c',
)

daemon_deps = [
    gtk,
    glib,
    gio,
    x11,
    json_glib,
]

executable('mate-hud-daemon',
    daemon_sources,
    dependencies : daemon_deps,
    install : true,
    install_dir : bindir,
)

helper_sources = files(
    'shortcut-helper.c',
)

executable('mate-hud-shortcut-helper',
    helper_sources,
    dependencies : daemon_deps,
    install : true,
    install_dir : bindir,
)
