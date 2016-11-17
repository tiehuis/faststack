cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(FastStack VERSION 0.2 LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")

add_subdirectory(src/engine)
add_compile_options(-Wall -Wextra -std=c99 -pedantic -O2)

option(use_sdl2 "Compile with the SDL2 frontend" ON)
option(use_terminal "Compile with the terminal frontend" OFF)

if (use_sdl2 AND use_terminal)
    message(FATAL_ERROR "Only one frontend can be chosen")
endif()

if (use_sdl2)
    message("building with SDL2 frontend")

    add_definitions(-DFS_USE_SDL2)
    add_executable(faststack
        src/main.c
        src/frontend/SDL2/frontend.c
        src/frontend/SDL2/deps/SDL_FontCache/SDL_FontCache.c
    )

    # TODO: Switch to using standard find_package sometime
    include(FindPkgConfig)

    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2_ttf REQUIRED SDL2_ttf)
    pkg_search_module(SDL2_mixer SDL2_mixer)

    include_directories(${SDL2_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${SDL2_LDFLAGS} ${SDL2_mixer_LDFLAGS} ${SDL2_ttf_LDFLAGS})
    target_include_directories(faststack PUBLIC src src/engine src/frontend/SDL2/deps/SDL_FontCache)

elseif (use_terminal)
    message("building with terminal frontend")

    add_definitions(-DFS_USE_TERMINAL)
    add_executable(faststack
        src/main.c
        src/frontend/terminal/frontend.c
    )

    target_include_directories(faststack PUBLIC src src/engine)

endif()

target_link_libraries(faststack libfs ${LIBS})
