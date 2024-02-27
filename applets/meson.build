if vala_panel_found
    vp_applets_libdir = vp.get_variable(pkgconfig:'applets_dir')
    vp_sources = files('valapanel-plugin-appmenu.vala')
    vpp = shared_module('appmenu', vp_sources, libres,
                    dependencies: [appmenu_dep, vp],
                    install: true,
                    install_dir: vp_applets_libdir
                  )
endif

if mate_found
    mate_applet_libdir = join_paths(mp.get_variable(pkgconfig:'libdir'),'mate-panel')
    mp_sources = files('mate-plugin-appmenu.vala')
    mpp = shared_module('appmenu-mate', mp_sources, libres,
                    dependencies: [appmenu_dep, mp],
                    install: true,
                    install_dir: mate_applet_libdir
                  )
endif

if xfce_found
    xfce_applet_libdir = join_paths(xp.get_variable(pkgconfig:'libdir'),'xfce4','panel','plugins')
    xp_sources = files('xfce4-plugin-appmenu.vala')
    xpp = shared_module('appmenu-xfce', xp_sources, libres,
                    dependencies: [appmenu_dep, xc,xp],
                    install: true,
                    install_dir: xfce_applet_libdir
                  )
endif

if budgie_found
    budgie_applet_libdir = join_paths(bp.get_variable(pkgconfig:'libdir'),'budgie-desktop','plugins','budgie-appmenu-plugin')
    bp_sources = files('budgie-plugin-appmenu.vala')
    bpp = shared_module('appmenu-budgie', bp_sources, libres,
                    dependencies: [appmenu_dep, bp],
                    install: true,
                    install_dir: budgie_applet_libdir
                  )
endif
