# We target the current Ubuntu 12.04 LTS version
cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(FastStack LANGUAGES C)

set(ROOT ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ROOT}/cmake")

# Compile Target

add_subdirectory(src/engine)
add_compile_options(-Wall -Wextra -std=c99 -pedantic -O2)

option(use_sdl2 "Compile with the SDL2 frontend" ON)
option(use_terminal "Compile with the terminal frontend" OFF)

if (use_sdl2 AND use_terminal)
    message(FATAL_ERROR "Only one frontend can be chosen")
endif()

if (use_sdl2)
    message("Frontend: SDL2")

    add_definitions(-DFS_USE_SDL2)
    add_executable(faststack
        src/main.c
        src/frontend/SDL2/frontend.c
        src/frontend/SDL2/deps/SDL_FontCache/SDL_FontCache.c
    )

    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer)

    include_directories(${SDL2_INCLUDE_DIR})
    set(LIBS ${LIBS} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES})
    target_include_directories(faststack PUBLIC src src/engine src/frontend/SDL2/deps/SDL_FontCache)

elseif (use_terminal)
    message("Frontend: Terminal")

    add_definitions(-DFS_USE_TERMINAL)
    add_executable(faststack
        src/main.c
        src/frontend/terminal/frontend.c
    )

    target_include_directories(faststack PUBLIC src src/engine)

endif()

target_link_libraries(faststack libfs ${LIBS})

# Post-Compile Target

# TODO: Move directory creation into actual executable + dont copy ini?
if (NOT EXISTS ${ROOT}/replay)
    file(MAKE_DIRECTORY replay)
endif()

if (NOT EXISTS ${ROOT}/fs.ini)
    file(COPY examples/fs.ini DESTINATION ${ROOT})
endif()

# Purge Target
add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -E remove -f ${ROOT}/faststack ${ROOT}/fs.ini ${ROOT}/fs.hiscore
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${ROOT}/replay
)
