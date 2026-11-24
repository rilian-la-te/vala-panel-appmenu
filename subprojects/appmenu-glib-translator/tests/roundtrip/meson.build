names = [
    'bad-children',
    'basic',
    'check',
    'dynamic',
    'exporter-rev-bump',
    'layout-updated',
    'nonexistent',
    'ordinary',
    'radio',
    'radio-bypass',
    'radio-selection',
    'submenu',
    'submenu-immutable-level1',
    'submenu-immutable-level2',
    'visibility',
    'insert'
]

foreach name : names
    exe = executable(name, name + '.c', test_main,
        dependencies: test_dep_lib,
    )
    test(name, exe, suite: 'roundtrip')
endforeach
