invasive_inc = include_directories('.')
invasive_dep = declare_dependency(
    sources: ['mock-server.c', 'mock-server.h', translator_dbus[1]],
    include_directories: invasive_inc,
    dependencies: [giounix, test_obj_dep]
)
