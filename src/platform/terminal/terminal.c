///
// FastStack-Terminal
//
// Linux Terminal frontend for the FastStack engine.
//
// We utilize the following potentially platform specific functions:
//
//  termios            - Removing cursors and handling input
//  vt100 escape codes - Terminal movement and colours
//  linux/input        - keyboard state querying through ioctl
///

#define _POSIX_C_SOURCE 199309L // For clock_gettime and nanosleep

#include <errno.h>              // Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <fcntl.h>              // Platform headers
#include <locale.h>
#include <linux/input.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>

#include "terminal.h"

// Set on sigwinch receive.
// We have a global here so we do not need to set our FSPSView as global.
static bool caughtSigwinch = false;

///
// Signal handler for handling a terminal resize event.
// If this occurs we must invalidate all buffers.
static void sigwinchHandler(int signal)
{
    (void) signal;
    caughtSigwinch = true;
}

static void initializeTerminal(FSPSView *v)
{
    // TODO: Determine how we can procedurally retrieve the keyboard device.
    static const char *inputDeviceName =
        "/dev/input/by-path/platform-i8042-serio-0-event-kbd";

    v->inputFd = open(inputDeviceName, O_RDONLY);

    // Handle access error as a special case
    if (v->inputFd == -1) {
        if (errno == EACCES) {
            fsLogFatal("Insufficient permission to open device: %s", inputDeviceName);
            fsLogFatal("Check the README for details to avoid this");
        } else {
            fsLogFatal("Failed to open input device: %s", strerror(errno));
        }

        exit(1);
    }

    // Keymap must be empty before being filled
    for (int i = 0; i < VKEY_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            v->keymap[i][j] = KEY_NONE;
        }
    }

    // Hide the cursor
    printf("\e[?25l");
    fflush(stdout);

    // Initialize sigwinch handler
    struct sigaction action = {0};
    action.sa_handler = sigwinchHandler;
    sigaction(SIGWINCH, &action, NULL);

    // First draw must be a complete redraw
    v->invalidateBuffers = true;

    // Set appropriate terminal settings
    tcgetattr(STDIN_FILENO, &v->initialTerminalState);
    struct termios ns = v->initialTerminalState;
    ns.c_lflag &= ~(ECHO | ICANON);
    ns.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &ns);
}

static void restoreTerminal(FSPSView *v)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &v->initialTerminalState);

    // Restore cursor
    printf("\e[?25h");
    fflush(stdout);

    if (close(v->inputFd) == -1) {
        fsLogError("Failed to close input device: %s", strerror(errno));
        exit(2);
    }
}

///
// Currently no sound is played.
void fsiPlaySe(FSPSView *v, FSBits se)
{
    (void) v;
    (void) se;
}

///
// Return the clock in microsecond granularity.
FSLong fsiGetTime(FSPSView *v)
{
    (void) v;

    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

///
// Sleep for the specified number of microseconds.
// TODO: Use clock_nanosleep instead.
void fsiSleepUs(FSPSView *v, FSLong time)
{
    (void) v;

    struct timespec rem;
    struct timespec req = { time / 1000000, 1000 * (time % 1000000) };

    // Assumptions: Any signals that interrupt this sleep do not take
    // excessively long to perform their operations.
    while (1) {
        // Successful sleep with no interruption, continue.
        if (nanosleep(&req, &rem) != -1)
            break;

        // Update the required sleep count and resleep.
        if (errno == EINTR) {
            req = rem;
        } else {
            fsLogError("Failure when calling nanosleep: %s", strerror(errno));
            exit(3);
        }
    }
}

///
// Return the found keypresses, performing the conversion from physical
// to virtual keys using the current keymap.
FSBits fsiReadKeys(FSPSView *v)
{
    char keystate[(KEY_MAX + 7) / 8] = {0};

    // We need to consume the characters that are pressed so they do not
    // dump on game end. Empty periodically so queue can't grow large.
    //
    // We use ioctl to query if any input is pending so we can perform
    // non-blocking reads.
    while (1) {
        int bp;
        ioctl(STDOUT_FILENO, FIONREAD, &bp);

        // If no bytes are pending stop getting input
        if (bp == 0)
            break;

        (void) getchar();
    }

    // Read physical key state
    ioctl(v->inputFd, EVIOCGKEY(sizeof(keystate)), keystate);

    // Check the status of all physical keys associated with each virtual key
    FSBits keys = 0;
    for (int i = 0; i < VKEY_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            // Keystate is stored as a bitset in the array of chars
            const int key = v->keymap[i][j];

            if (keystate[key >> 3] & (1 << (key & 7))) {
                // This is valid since we have a bit-mask associated
                // with each index. Make a macro here though.
                keys |= (1 << i);
            }
        }
    }

    return keys;
}

///
// Return the colour attribute for the specified piece.
static uint16_t attr_colour(int piece)
{
    static const int attrmap[] = {
        ATTR_CYAN,      // I
        ATTR_BLUE,      // J
        ATTR_WHITE,     // L
        ATTR_YELLOW,    // O
        ATTR_GREEN,     // S
        ATTR_MAGENTA,   // T
        ATTR_RED        // Z
    };

    if (piece < 0 || FS_NPT < piece) {
        fsLogError("Invalid piece type passed to attr_colour: %d", piece);
        exit(1);
    }

    return attrmap[piece];
}

// Draw the hold piece buffer.
static void drawHold(FSPSView *v)
{
    FSInt2 blocks[4];
    const FSGame *f = v->view->game;

    if (f->holdPiece == FS_NONE)
        return;

    fsPieceToBlocks(f, blocks, f->holdPiece, 0, 0, 0);
    for (int i = 0; i < FS_NBP; ++i) {
        const int xoffset = f->holdPiece == FS_I || f->holdPiece == FS_O ? 0 : 1;

        v->bbuf[HOLD_Y + blocks[i].y][HOLD_X + 2*blocks[i].x + xoffset] = (TerminalCell) {
            .value = GLYPH_LBLOCK,
            .attrs = ATTR_REVERSE | attr_colour(f->holdPiece)
        };
        v->bbuf[HOLD_Y + blocks[i].y][HOLD_X + 2*blocks[i].x + 1 + xoffset] = (TerminalCell) {
            .value = GLYPH_RBLOCK,
            .attrs = ATTR_REVERSE | attr_colour(f->holdPiece)
        };
    }
}

// Draw the main field state. This is everything with the bounding box of
// the actual field.
static void drawField(FSPSView *v)
{
    FSInt2 blocks[4];
    const FSGame *f = v->view->game;

    ///
    // Border
    v->bbuf[FIELD_Y + f->fieldHeight][FIELD_X + 1].value = GLYPH_LWALL_FLOOR;
    v->bbuf[FIELD_Y + f->fieldHeight][FIELD_X + 2*f->fieldWidth + 2].value = GLYPH_RWALL_FLOOR;

    for (int y = 0; y < f->fieldHeight; ++y) {
        v->bbuf[FIELD_Y + y][FIELD_X + 1].value = GLYPH_WALL;
        v->bbuf[FIELD_Y + y][FIELD_X + 2*f->fieldWidth + 2].value = GLYPH_WALL;
    }

    for (int x = 0; x < 2 * f->fieldWidth; ++x) {
        v->bbuf[FIELD_Y + f->fieldHeight][FIELD_X + x + 2].value = GLYPH_FLOOR;
    }

    ///
    // Field state
    for (int y = 0; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            const TerminalCell sq = (TerminalCell) {
                .value = GLYPH_EMPTY,
                .attrs = f->b[y][x] ? ATTR_REVERSE | ATTR_WHITE  : 0
            };

            v->bbuf[FIELD_Y + y][FIELD_X + 2*x + 2] = sq;
            v->bbuf[FIELD_Y + y][FIELD_X + 2*x + 3] = sq;
        }
    }

    // Only print piece if valid
    if (f->piece != FS_NONE) {
        ///
        // Current piece ghost
        fsPieceToBlocks(f, blocks, f->piece, f->x, f->hardDropY, f->theta);
        for (int i = 0; i < FS_NBP; ++i) {
            v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 2] = (TerminalCell) {
                .value = GLYPH_LBLOCK,
                .attrs = ATTR_REVERSE | ATTR_DIM | attr_colour(f->piece)
            };
            v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 3] = (TerminalCell) {
                .value = GLYPH_RBLOCK,
                .attrs = ATTR_REVERSE | ATTR_DIM | attr_colour(f->piece)
            };
        }

        ///
        // Current piece
        fsPieceToBlocks(f, blocks, f->piece, f->x, f->y, f->theta);
        for (int i = 0; i < FS_NBP; ++i) {
            v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 2] = (TerminalCell) {
                .value = GLYPH_LBLOCK,
                .attrs = ATTR_REVERSE | attr_colour(f->piece)
            };
            v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 3] = (TerminalCell) {
                .value = GLYPH_RBLOCK,
                .attrs = ATTR_REVERSE | attr_colour(f->piece)
            };
        }
    }
}

///
// Draw preview piece buffer.
static void drawPreview(FSPSView *v)
{
    FSInt2 blocks[FS_NBP];
    const FSGame *f = v->view->game;

    // Not printing correctly!
    const int previewCount = f->nextPieceCount > 4 ? 4 : f->nextPieceCount;

    // Print 4 preview pieces max for now (where do we render if higher?)
    for (int i = 0; i < previewCount; ++i) {
        fsPieceToBlocks(f, blocks, f->nextPiece[i], 0, 0, 0);
        const int xpo = f->nextPiece[i] == FS_I || f->nextPiece[i] == FS_O ? 0 : 1;

        for (int j = 0; j < FS_NBP; ++j) {
            const int xo = PVIEW_X + xpo + 2 * blocks[j].x;
            const int yo = PVIEW_Y + (4 * i) + blocks[j].y;

            v->bbuf[yo][xo] = (TerminalCell) {
                .value = GLYPH_LBLOCK,
                .attrs = ATTR_REVERSE | attr_colour(f->nextPiece[i])
            };
            v->bbuf[yo][xo + 1] = (TerminalCell) {
                .value = GLYPH_RBLOCK,
                .attrs = ATTR_REVERSE | attr_colour(f->nextPiece[i])
            };
        }
    }
}

///
// Copy a string into the backbuffer at the specified coordinates.
static void putStrAt(FSPSView *v, const char *s, int y, int x, int attrs)
{
    if (y < 0 || FS_TERM_HEIGHT <= y || x < 0 || FS_TERM_WIDTH <= x)
        return;

    const int slen = strlen(s);
    for (int i = 0; i < slen; ++i) {
        // Truncate edges that are too long
        if (x + i > FS_TERM_WIDTH)
            break;

        v->bbuf[y][x + i] = (TerminalCell) {
            .value = s[i],
            .attrs = attrs
        };
    }
}

///
// Draw info segment.
static void drawInfo(FSPSView *v)
{
    const FSGame *f = v->view->game;

    // Limit this to the specified length for now
    const int bufsiz = 32;
    char buf[bufsiz];

    // Target beneath field
    int remaining = v->view->game->goal - v->view->game->linesCleared;
    if (remaining < 0)
        remaining = 0;

    snprintf(buf, bufsiz, "%d", remaining);
    putStrAt(v, buf, FIELD_Y + FIELD_H + 1,
                FIELD_X + FIELD_W / 2 - strlen(buf) / 2 + 1, ATTR_BRIGHT);

    const int msElapsed = f->msPerTick * f->totalTicks;

    snprintf(buf, bufsiz, "Time");
    putStrAt(v, buf, INFO_Y + 1, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%.3f", (float) msElapsed / 1000);
    putStrAt(v, buf, INFO_Y + 2, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "Blocks Placed");
    putStrAt(v, buf, INFO_Y + 4, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%d", f->blocksPlaced);
    putStrAt(v, buf, INFO_Y + 5, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "Tetriminos Per Second (TPS)");
    putStrAt(v, buf, INFO_Y + 7, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%.5f",
             msElapsed != 0
                ? (float) f->blocksPlaced / ((float) msElapsed / 1000)
                : 0);
    putStrAt(v, buf, INFO_Y + 8, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "Finesse");
    putStrAt(v, buf, INFO_Y + 10, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%d", f->finesse);
    putStrAt(v, buf, INFO_Y + 11, INFO_X, ATTR_BRIGHT);
}

void fsiBlit(FSPSView *v)
{
    // If sigwinch was caught invalidate buffers (kind of pointless doing this
    // set but it may be changed in the future).
    if (caughtSigwinch) {
        v->invalidateBuffers = true;
        caughtSigwinch = false;
    }

    // Clear entire screen if invalid buffers and restart
    if (v->invalidateBuffers)
        printf("\e[H\e[2J");

    for (int y = 0; y < FS_TERM_HEIGHT; ++y) {
        for (int x = 0; x < FS_TERM_WIDTH; ++x) {
            if (v->invalidateBuffers
                    || v->bbuf[y][x].value != v->fbuf[y][x].value
                    || v->bbuf[y][x].attrs != v->fbuf[y][x].attrs) {
                // Apply all attributes for the current block (only if non-zero)
                if (v->bbuf[y][x].attrs) {
                    for (int i = 0; i < ATTR_COUNT; ++i) {
                        if (v->bbuf[y][x].attrs & (1 << i)) {
                            printf("\e[%dm", attributes[i]);
                        }
                    }
                }

                // Print the actual value
                printf("\e[%d;%dH", y + 1, x + 1);
                printf("%c", (char) v->bbuf[y][x].value);

                // Reset attributes for the piece
                printf("\e[0m");
            }

            v->fbuf[y][x] = v->bbuf[y][x];
        }
    }

    // We need to flush since we don't print any newlines
    fflush(stdout);
    v->invalidateBuffers = false;
}

void fsiAddToKeymap(FSPSView *v, int virtualKey, const char *keyValue)
{
    const int kc = fsKeyToPhysicalKey(keyValue);
    if (kc) {
        for (int i = 0; i < FS_MAX_KEYS_PER_ACTION; ++i) {
            // Found an empty slot to fill
            if (v->keymap[virtualKey][i] == KEY_NONE) {
                v->keymap[virtualKey][i] = kc;
                return;
            }
        }

        // Keymap was full, warn user
        fsLogWarning("Could not insert key %s into full keymap", keyValue);
    }
}

// parse ini will call this function and pass appropriate values
void fsiUnpackFrontendOption(FSPSView *v, const char *key, const char *value)
{
    (void) v;
    (void) value;

    if (!strncmp(key, "frontend.terminal.", 18)) {
        // Handle other specific options
    }
}

// Peform an entire draw, blit loop
void fsiDraw(FSPSView *v)
{
    // Empty backbuffer for redraw
    for (int y = 0; y < FS_TERM_HEIGHT; ++y) {
        for (int x = 0; x < FS_TERM_WIDTH; ++x) {
            v->bbuf[y][x] = (TerminalCell) {
                .value = ' ',
                .attrs = 0
            };
        }
    }

    drawField(v);
    drawHold(v);
    drawPreview(v);
    drawInfo(v);
}

// Run before any user code is processed in a tick
void fsiPreFrameHook(FSPSView *v)
{
    (void) v;
}

// Run after we have slept for the specified period of time
void fsiPostFrameHook(FSPSView *v)
{
    const FSGame *f = v->view->game;

    switch (f->state) {
      case FSS_READY:
      case FSS_GO:
      {
        int i = 0;
        char *ready = "ready", *go = "go";
        for (char *s = f->state == FSS_READY ? ready : go; *s != '\0'; s++)
            v->bbuf[FIELD_Y + f->fieldHeight / 2][FIELD_X + f->fieldWidth + i++].value = *s;
        break;
      }
      default:
        break;
    }
}

int main(void)
{
    FSGame game;
    FSControl control;
    // Generic View
    FSView gView = { .game = &game, .control = &control, .totalFramesDrawn = 0 };
    // Platform-Specific View
    FSPSView pView = { .view = &gView };

    initializeTerminal(&pView);

    fsGameInit(&game);
    fsParseIniFile(&pView, &gView, "fs.ini");
    fsGameLoop(&pView, &gView);

    // Give some space after the screen so we can still view it nicely after a
    // game has completed.
    // Cursor is ALWAYS at the end after a draw so we are in the correct spot
    // to print.
    printf("\n\n");

    restoreTerminal(&pView);
}
