project('faststack', 'c',
    version : '0.3.0',
    license : 'GPLv3'
)

add_project_arguments(['-Wall', '-Wextra', '-std=c99', '-pedantic'],
    language : 'c'
)

subdir('src/engine')
src = ['src/main.c']
inc = [engine_inc]
deps = []
defines = []

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
