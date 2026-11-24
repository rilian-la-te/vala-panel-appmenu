invasive_inc = include_directories('.')
invasive_dep = declare_dependency(
    sources: ['mock-server.c', 'mock-server.h', translator_dbus[1]],
    include_directories: invasive_inc,
    dependencies: [giounix, test_obj_dep]
)

invasive_fixture = static_library(
    'invasive-fixture',
    'invasive-fixture.c', 'invasive-fixture.h',
    include_directories: [invasive_inc, translator_inc],
    dependencies: invasive_dep,
)
invasive_fixture_dep = declare_dependency(
    link_with: invasive_fixture,
    include_directories: [invasive_inc, translator_inc],
    dependencies: invasive_dep,
)

names = [
    'dummy-labels-only',
    'separator-run'
]

foreach name : names
    exe = executable(name, name + '.c', test_main,
        dependencies: invasive_dep
    )
    test(name, exe, suite: 'invasive')
endforeach
