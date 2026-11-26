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
    'kde-title',
    'kde-title-relabelled',
    'kde-title-cleared',
    'kde-title-two-items',
    'kde-title-shrink',
    'kde-title-run',
    'kde-title-hidden',
    'kde-title-leading',
    'kde-title-leading-cleared',
    'kde-title-leading-hidden',
    'dummy-labels-only',
    'firefox-stubs',
    'separator-run'
]

foreach name : names
    exe = executable(name, name + '.c', test_main,
        dependencies: invasive_fixture_dep
    )
    test(name, exe, suite: 'invasive')
endforeach
