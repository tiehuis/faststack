test_deps = []
test_src = ['test_randomizer.c']
test_defines = ['-DFS_DISABLE_OPTION']

test_randomizer = executable('test_randomizer',
    test_src,
    c_args : test_defines,
    include_directories : engine_inc,
    link_with : engine_lib
)

test('randomizer', test_randomizer)
