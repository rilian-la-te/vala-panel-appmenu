module_sources = files(
    'appmenu-gtk-module.c',
    'datastructs.c',
    'datastructs.h',
    'datastructs-private.h',
    'hijack.c',
    'hijack.h',
    'support.c',
    'support.h',
    'blacklist.c',
    'blacklist.h',
    'platform.c',
    'platform.h',
    'consts.h'
)

x11_sources = files(
    'platform-x11.c'
)

if wayland_requested
    wl_sources = files(
        'platform-gtk-shell.c'
    )
else
    wl_sources = []
endif

module_subdirs = ['gtk-2.0','gtk-3.0']

if build_gtk2
    subdir('gtk-2.0')
endif

if build_gtk3
    subdir('gtk-3.0')
endif
