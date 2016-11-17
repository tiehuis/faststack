add_compile_options(-Wall -Wextra -std=c99 -pedantic -O2)
add_library(libfs STATIC
    control.c
    engine.c
    finesse.c
    hiscore.c
    keyspec.c
    log.c
    option.c
    rand.c
    replay.c
    rotation.c
)
