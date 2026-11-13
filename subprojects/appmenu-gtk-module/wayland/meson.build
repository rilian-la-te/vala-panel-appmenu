if wl_dep.found()
    wl_mod = import('wayland')

    decor = wl_mod.find_protocol(
    'xdg-decoration',
    state : 'unstable',
    version : 1,
    )

    shell = wl_mod.find_protocol(
    'xdg-shell',
    state : 'stable'
    )

    protocols = [
        decor,
        shell,
        'org.kde.appmenu.xml',
        'gtk-shell.xml'
    ]

    generated = wl_mod.scan_xml(
    protocols,
    client : true,
    server : false,
    public : false,
    )

    wayland_lib = static_library('libappmenu-wayland',
            generated, 'protocol-check.c',
            dependencies: wl_dep,
            pic: true
        )
    wayland_dep = declare_dependency(
        link_with: wayland_lib,
        include_directories: include_directories('.'),
        dependencies: wl_dep
    )
else
    wayland_dep = []
endif
