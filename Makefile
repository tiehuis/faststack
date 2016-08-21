CC		?= clang
CFLAGS  += -Wall -Wextra -std=c99 -O2 -Isrc
CDFLAGS += -g
LDFLAGS += -lm

PROGRAM := FastStack
SOURCE  := src/*.c

SDL2_SOURCE := src/platform/sdl2/*.c
SDL2_OPTION := `pkg-config sdl2 SDL2_mixer SDL2_ttf --cflags --libs`

TERM_SOURCE := src/platform/terminal/*.c
TERM_OPTION :=


default: sdl2

sdl2:
	${CC} ${CFLAGS} ${CDFLAGS} -o ${PROGRAM} ${SOURCE} ${SDL2_SOURCE} ${SDL2_OPTION} ${LDFLAGS}

term:
	${CC} ${CFLAGS} ${CDFLAGS} -o ${PROGRAM} ${SOURCE} ${TERM_SOURCE} ${TERM_OPTION} ${LDFLAGS}

clean:
	rm -f ${PROGRAM}

.phony:
	clean
