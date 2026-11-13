
test_library = static_library('test_translator', files('comparator.c'), dependencies: translator_dep)

names = [
    'bad-children',
    'basic',
    'check',
    'dynamic',
    'radio',
    'radio-bypass',
    'radio-selection',
    'submenu',
    'insert'
]

foreach name : names
    exe = executable(name, name + '-test.c', 'test-main.c',
        dependencies: translator_dep,
        link_whole: test_library,
    )
    test(name, exe)
endforeach