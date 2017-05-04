sdl_src = files(['frontend.c', 'deps/SDL_FontCache/SDL_FontCache.c'])
sdl_deps = [
    dependency('sdl2', required : true),
    dependency('SDL2_ttf', required : true)
]
sdl_defines = ['-DFS_USE_SDL2']
sdl_inc = include_directories('deps/SDL_FontCache')
