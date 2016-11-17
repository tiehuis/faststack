CC      ?= clang
CFLAGS  += -Wall -Wextra -std=c99 -pedantic -O2 -Isrc -Isrc/engine ${EXTRA}
CDFLAGS += -g
LDFLAGS +=

PROGRAM := faststack
SOURCE  := src/engine/*.c src/*.c

SDL2_SOURCE := src/frontend/SDL2/*.c src/frontend/SDL2/deps/SDL_FontCache/*.c
SDL2_OPTION := `pkg-config sdl2 SDL2_mixer SDL2_ttf --cflags --libs` -isystem src/frontend/SDL2/deps/SDL_FontCache

TERM_SOURCE := src/frontend/terminal/*.c
TERM_OPTION :=


default: sdl2

sdl2:
	${CC} -DFS_USE_SDL2 ${CFLAGS} ${CDFLAGS} -o ${PROGRAM} ${SOURCE} \
		${SDL2_SOURCE} ${SDL2_OPTION} ${LDFLAGS}

terminal:
	${CC} -DFS_USE_TERMINAL ${CFLAGS} ${CDFLAGS} -o ${PROGRAM} ${SOURCE} \
		${TERM_SOURCE} ${TERM_OPTION} ${LDFLAGS}

clean:
	rm -f ${PROGRAM}

.phony:
	clean
