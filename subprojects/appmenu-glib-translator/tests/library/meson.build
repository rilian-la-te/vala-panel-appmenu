test_library = static_library('test_translator', files('comparator.c'), dependencies: translator_dep)
test_inc = include_directories('.')
test_dep_lib = declare_dependency(
    include_directories: test_inc,
    link_with: test_library,
    dependencies: [giounix, translator_dep]
)

translator_obj = translator_lib.extract_all_objects(recursive: false)
test_obj = test_library.extract_all_objects(recursive: false)
test_obj_dep = declare_dependency(
    include_directories: [test_inc] + translator_inc,
    objects: [translator_obj, test_obj],
    dependencies: giounix
)
test_main = files('test-main.c')
