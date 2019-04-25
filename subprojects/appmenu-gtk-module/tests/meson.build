if build_gtk3 and get_option('tests')
    client_sources = files (
	    join_paths('demos','altgrabber.h'),
	    join_paths('demos','altgrabber.c'),
	    join_paths('demos','altmonitor.h'),
	    join_paths('demos','altmonitor.c'),
	    join_paths('demos','menusource.h'),
	    join_paths('demos','menusource.c'),
	    join_paths('demos','client.c'))

    wnck_ver = '>=3.4.8'
    wnck = dependency('libwnck-3.0',version: wnck_ver, required: false)
    x11 = dependency('x11', required: false)
    if(wnck.found() and x11.found())
        client = executable('menumodel-client',client_sources,
            dependencies: [gtk3_parser_dep, wnck, x11]
        )
#        test('client', client)
    endif
    tester = executable('tester',join_paths('demos','unity-gtk-menu-tester.c'), dependencies: gtk3)
#    test('tester',tester)
    radio = executable('radio',join_paths('demos','test-radio.c'), dependencies: gtk3)
#    test('radio',radio)
    hello = executable('hello',join_paths('demos','hello.c'), dependencies: gtk3)
#    test('hello',hello)
    vala_found = add_languages('vala', required: false)
    if vala_found
        black = executable('black',join_paths('demos','black.vala'), dependencies: gtk3)
#        test('black',black)
    endif
endif

#TODO: fix autopilot
run_target('autopilot',
        command: ['autopilot/test.sh'],
    )
