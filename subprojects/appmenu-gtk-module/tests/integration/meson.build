
test_deps = [gtk3, importer_dep, gtk3_parser_dep]

test_library = static_library('test_gtk_module', files('comparator.c', 'test-helper.c'), dependencies: test_deps, include_directories: include_directories('.'))

fs = import('fs')

roundtrip_sources = files(
  'menu-roundtrip-static.c',
  'menu-roundtrip-radio.c'
)

foreach src : roundtrip_sources
  name = fs.stem(src.full_path())

  rt = executable(name, src,
    dependencies: test_deps,
    link_whole: test_library,
  )
  test(name, rt)

  rt_nolazy = executable(name + '-nolazy', src,
    dependencies: test_deps,
    link_whole: test_library,
    c_args: ['-DNO_LAZY'],
  )
  test(name + '-nolazy', rt_nolazy)
endforeach

names = [
    'checkbox-source',
    'checkbox-target',
    'radio-source',
    'radio-target',
    'dynamic-bypass',
    'regular-activate'
]

foreach name : names
    exe = executable(name, name + '.c', 'test-main.c',
        dependencies: test_deps,
        link_whole: test_library,
    )
    test(name, exe)
endforeach