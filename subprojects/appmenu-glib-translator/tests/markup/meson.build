# It is a tests for GMenuModel<->XML converter
names = [
    'markup-parse',
    'ui-file-parse',
    'interface-wrap',
    'id-lookup'
]

foreach name : names
    exe = executable(name, name + '.c', test_main,
        dependencies: test_dep_lib,
    )
    test(name, exe, suite: 'markup')
endforeach
