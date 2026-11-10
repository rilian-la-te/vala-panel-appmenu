
test_library = static_library('test_translator', files('comparator.c'), dependencies: translator_dep)

basic = executable('basic', 'basic-test.c',
    dependencies: translator_dep,
    link_whole: test_library,
)
test('basic', basic)

radio = executable('radio', 'radio-test.c',
    dependencies: translator_dep,
    link_whole: test_library,
)
test('radio', radio)
