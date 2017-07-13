project('faststack', 'c',
    version : '0.3.1',
    license : 'GPLv3'
)

add_project_arguments(['-Wall', '-Wextra', '-std=c99', '-pedantic'],
    language : 'c'
)

if meson.is_cross_build()
    add_project_link_arguments(['-L../dlls'], language : 'c')
endif

subdir('src/engine')

src = []
inc = [engine_inc]
deps = []
defines = []

if get_option('disable-option')
    defines += ['-DFS_DISABLE_OPTION']
endif
if get_option('disable-log')
    defines += ['-DFS_DISABLE_LOG']
endif
if get_option('disable-replay')
    defines += ['-DFS_DISABLE_REPLAY']
endif
if get_option('disable-hiscore')
    defines += ['-DFS_DISABLE_HISCORE']
endif

frontend = get_option('frontend')
if frontend == 'sdl'
    subdir('src/frontend/SDL2')
    src += sdl_src
    deps += sdl_deps
    defines += sdl_defines
    inc += sdl_inc
elif frontend == 'terminal'
    subdir('src/frontend/terminal')
    src += terminal_src
    deps += terminal_deps
    defines += terminal_defines
    inc += terminal_inc
else
    error('invalid frontend specified')
endif

executable('faststack',
    src,
    include_directories : inc,
    c_args : defines,
    link_with : engine_lib,
    dependencies : deps
)

subdir('test')

