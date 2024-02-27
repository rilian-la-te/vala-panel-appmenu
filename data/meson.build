cdata = configuration_data()
cdata.set('CMAKE_INSTALL_FULL_LIBDIR', join_paths(prefix,get_option('libdir')))
cdata.set('CMAKE_INSTALL_FULL_DATAROOTDIR', join_paths(prefix,datadir))

if vala_panel_found
    vp_applets_datadir = vp.get_variable(pkgconfig:'applets_data')
    desktop = vcs_tag(
	    command : ['git','describe','--tags'],
	    input: 'appmenu.plugin.desktop.in',
	    output: 'appmenu.plugin.in',
	    replace_string: '@VERSION@',
    )
    i18n.merge_file(
      input: desktop,
      output: 'org.valapanel.appmenu.plugin',
      kwargs: desktop_kwargs,
      install_dir: vp_applets_datadir
    )
endif

if mate_found
    mate_applet_datadir = join_paths(mp.get_variable(pkgconfig:'prefix'),datadir,'mate-panel','applets')
    mate_desktop = configure_file(input : 'appmenu-mate.plugin.desktop.in',
               output : 'appmenu-mate.desktop.in',
			   configuration : cdata)
    i18n.merge_file(
      input: mate_desktop,
      output: 'org.vala-panel.appmenu.mate-panel-applet',
      kwargs: desktop_kwargs,
      install_dir: mate_applet_datadir
    )
endif

if xfce_found
    xfce_applet_datadir = join_paths(xp.get_variable(pkgconfig:'prefix'),datadir,'xfce4','panel','plugins')
    i18n.merge_file(
      input: 'appmenu.desktop.in',
      output: 'appmenu.desktop',
      kwargs: desktop_kwargs,
      install_dir: xfce_applet_datadir
    )
endif

if budgie_found
    i18n.merge_file(
      input: 'appmenu-budgie.desktop.in',
      output: 'appmenu-budgie.plugin',
      kwargs: desktop_kwargs,
      install_dir: budgie_applet_libdir
    )
endif

if mate_found or vala_panel_found or budgie_found
    install_data([
	    'org.valapanel.appmenu.gschema.xml'
    ], install_dir: schema_dir)
endif
