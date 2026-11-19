
test_library = static_library('test_translator', files('comparator.c'), dependencies: translator_dep)

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
    'visibility',
    'insert'
]

foreach name : names
    exe = executable(name, name + '.c', 'test-main.c',
        dependencies: translator_dep,
        link_whole: test_library,
    )
    test(name, exe)
endforeach
