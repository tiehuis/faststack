PROGRAM := a.out
SOURCE  := src/*.c src/platform/sdl2/*.c
CC		?= clang

CFLAGS  += -Wall -Wextra
CDFLAGS += -g
CBFLAGS += -O2 -march=native
LDFLAGS +=

debug:
	${CC} ${CFLAGS} ${CDFLAGS} -o ${PROGRAM} ${SOURCE} `pkg-config sdl2 SDL2_ttf --cflags --libs`

build:
	${CC} ${CFLAGS} ${CBFLAGS} -o ${PROGRAM} ${SOURCE} `pkg-config sdl2 SDL2_ttf --cflags --libs`
	strip -S ${PROGRAM}

term:
	${CC} ${CFLAGS} ${CBFLAGS} -g -o ${PROGRAM} src/*.c src/platform/terminal/*.c
