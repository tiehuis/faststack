///
// frontend.c
// ==========
//
// Linux Terminal frontend for the faststack engine.
//
// The following platform-specific functions are used:
//  * Termios
//  * VT100 Escape Codes
//  * linux/input header
///

#define _POSIX_C_SOURCE 199309L // For clock_gettime and nanosleep
#define _XOPEN_SOURCE 500       // For SA_RESETHAND

#include "frontend.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <linux/input.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>

const char *fsiFrontendName = "terminal";

volatile sig_atomic_t caughtSigwinch = 0;
volatile sig_atomic_t caughtSigint = 0;

static const int attributes[] = {
    // Attribute masks index into this (i) not (1 << i)
    7, 30, 31, 32, 33, 34, 35, 36, 37,
    4, 1, 2, 5
};

static void sigwinchHandler(int signal)
{
    (void) signal;
    caughtSigwinch = 1;
}

static void sigintHandler(int signal)
{
    (void) signal;
    caughtSigint = 1;
}

void fsiPreInit(FSFrontend *v)
{
    // We must explicitly clear the keymap else garbage keys could be pressed.
    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            v->keymap[i][j] = (KeyEntry) {
                .isDefault = false,
                .value = KEY_NONE
            };
        }
    }

    // Default to ascii. Allow overwrite on ini load.
    v->glyph = asciiGlyphSet;
}

#define PROC_DEVICE_FILENAME "/proc/bus/input/devices"
#define DEV_INPUT_PREFIX "/dev/input"

// This should work across most linux machines (untested).
static int openInputDevice(void)
{
    char buf[128] = {0};
    char deviceName[10] = {0};
    char evTypes[10] = {0};

    FILE *fd = fopen(PROC_DEVICE_FILENAME, "rb");
    if (fd == NULL) { printf("failed to open device\n"); return 0; }

    while (1) {
        // Mark the end of the buffer so we can determine excessive lines
        buf[sizeof(buf) - 1] = 'Z';

        if (fgets(buf, sizeof(buf), fd) == NULL) {
            break;
        }

        // Discard long lines since they were likely uninteresting
        if (buf[sizeof(buf) - 1] == '\0' && buf[sizeof(buf) - 2] != '\n') {
            int ch;
            while ((ch = fgetc(fd)) != '\n' && ch != EOF) {}
        }

        if (buf[0] == '\n') {
            if ((evTypes[0] == '1') &&
                (evTypes[1] == '0' || evTypes[1] == '2') &&
                (evTypes[2] == '0') &&
                (evTypes[3] == '0') &&
                (evTypes[4] == '1') &&
                (evTypes[5] == '3' || evTypes[5] == 'F' || evTypes[5] == 'f'))
            {
                fclose(fd);

                fsLogInfo("determined input device to be %s", deviceName);
                snprintf(buf, sizeof(buf), DEV_INPUT_PREFIX "/" "%s", deviceName);
                const int fd = open(buf, O_RDONLY);

                if (fd == -1) {
                    if (errno == EACCES) {
                        fsLogFatal("Insufficient permission to open device: %s",
                                   deviceName);
                        fsLogFatal("Try adding yourself to the group 'input'");
                    } else {
                        fsLogFatal("Failed to open input device: %s", strerror(errno));
                    }

                    exit(1);
                }

                return fd;
            }

            evTypes[0] = '\0';
            deviceName[0] = '\0';
        }
        else {
            if (!strncmp(buf, "H: Handlers=", 12)) {
                const char *st = strstr(buf + 12, "event");
                if (st) {
                    char *en = strpbrk(st, " \n");
                    if (en) {
                        *en = '\0';
                    }

                    strncpy(deviceName, st, sizeof(deviceName));
                }
            }
            else if (!strncmp(buf, "B: EV=", 6)) {
                strncpy(evTypes, buf + 6, sizeof(evTypes));
            }
        }
    }

    // Note: Can be overridden by an option?
    fclose(fd);
    fsLogFatal("Could not find an input device!");
    exit(1);
}

void fsiInit(FSFrontend *v)
{
    v->inputFd = openInputDevice();

    // Clear the cursor
    printf("\033[?25l");
    fflush(stdout);

    struct sigaction action = {0};
    action.sa_handler = sigwinchHandler;
    sigaction(SIGWINCH, &action, NULL);

    action.sa_handler = sigintHandler;
    action.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &action, NULL);

    tcgetattr(STDIN_FILENO, &v->initialTerminalState);
    struct termios ns = v->initialTerminalState;
    ns.c_lflag &= ~(ECHO | ICANON);
    ns.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &ns);

    v->invalidateBuffers = true;
}

void fsiFini(FSFrontend *v)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &v->initialTerminalState);

    // Show the cursor
    printf("\033[?25h");
    printf("\033[%d;%dH", FS_TERM_HEIGHT, FS_TERM_WIDTH);

    // Cursor is guaranteed to be at the end of the screen, so print some extra
    // lines on exit to better display score.
    printf("\n\n");

    if (close(v->inputFd) == -1) {
        fsLogError("Failed to close input device: %s", strerror(errno));
        exit(2);
    }
}

void fsiPlaySe(FSFrontend *v, u32 se)
{
    (void) v;
    (void) se;
}

///
// A monotonic clock with microsecond granularity.
//
// Notes:
//  * Should use CLOCK_MONOTONIC_RAW where available.
i32 fsiGetTime(FSFrontend *v)
{
    (void) v;

    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

///
// Sleep for the specified number of microseconds.
//
// Notes:
//  * Should clock_nanosleep instead for consistency.
void fsiSleep(FSFrontend *v, i32 time)
{
    (void) v;

    struct timespec rem;
    struct timespec req = { time / 1000000, 1000 * (time % 1000000) };

    // We assume that any signal interruptions do not take excessively long
    // to perform their operations.
    while (1) {
        // Successful sleep with no interruption, continue.
        if (nanosleep(&req, &rem) != -1) {
            break;
        }

        // Update remaining sleep count and continue.
        if (errno == EINTR) {
            req = rem;
        } else {
            fsLogError("Failure when calling nanosleep: %s", strerror(errno));
            exit(3);
        }
    }
}

///
// Return the found keypresses.
//
// Notes:
//  * Can we replace the required getchar() with an ioctl call to mute the
//    output on this terminal?
u32 fsiReadKeys(FSFrontend *v)
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
        if (bp == 0) {
            break;
        }

        (void) getchar();
    }

    // Fills buffer with current keystate
    ioctl(v->inputFd, EVIOCGKEY(sizeof(keystate)), keystate);

    u32 keys = 0;
    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            // Keystate is stored as a bitset in the array of chars
            const int key = v->keymap[i][j].value;
            if (key == KEY_NONE) {
                break;
            }

            if (keystate[key >> 3] & (1 << (key & 7))) {
                keys |= FS_TO_FLAG(i);
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

static void drawHold(FSFrontend *v)
{
    i8x2 blocks[4];
    const FSEngine *f = v->view->game;

    if (f->holdPiece == FS_NONE) {
        return;
    }

    fsGetBlocks(f, blocks, f->holdPiece, 0, 0, 0);
    for (int i = 0; i < FS_NBP; ++i) {
        const int xoffset = f->holdPiece == FS_I || f->holdPiece == FS_O ? 0 : 1;

        v->bbuf[HOLD_Y + blocks[i].y][HOLD_X + 2*blocks[i].x + xoffset] = (TerminalCell) {
            .value = v->glyph.blockL,
            .attrs = ATTR_REVERSE | attr_colour(f->holdPiece)
        };
        v->bbuf[HOLD_Y + blocks[i].y][HOLD_X + 2*blocks[i].x + 1 + xoffset] = (TerminalCell) {
            .value = v->glyph.blockR,
            .attrs = ATTR_REVERSE | attr_colour(f->holdPiece)
        };
    }
}

static void drawField(FSFrontend *v)
{
    i8x2 blocks[4];
    const FSEngine *f = v->view->game;

    ///
    // Border
    v->bbuf[FIELD_Y + f->fieldHeight - f->fieldHidden][FIELD_X + 1].value = v->glyph.borderLB;
    v->bbuf[FIELD_Y + f->fieldHeight - f->fieldHidden][FIELD_X + 2*f->fieldWidth + 2].value = v->glyph.borderRB;

    for (int y = 0; y < f->fieldHeight - f->fieldHidden; ++y) {
        v->bbuf[FIELD_Y + y][FIELD_X + 1].value = v->glyph.borderL;
        v->bbuf[FIELD_Y + y][FIELD_X + 2*f->fieldWidth + 2].value = v->glyph.borderR;
    }

    for (int x = 0; x < 2 * f->fieldWidth; ++x) {
        v->bbuf[FIELD_Y + f->fieldHeight - f->fieldHidden][FIELD_X + x + 2].value = v->glyph.borderB;
    }

    ///
    // Field state
    for (int y = f->fieldHidden; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            const TerminalCell sq = (TerminalCell) {
                .value = v->glyph.blockE,
                .attrs = f->b[y][x] ? ATTR_REVERSE | ATTR_WHITE  : 0
            };

            v->bbuf[FIELD_Y + (y - f->fieldHidden)][FIELD_X + 2*x + 2] = sq;
            v->bbuf[FIELD_Y + (y - f->fieldHidden)][FIELD_X + 2*x + 3] = sq;
        }
    }

    if (f->piece == FS_NONE) {
        return;
    }

    ///
    // Current piece ghost
    fsGetBlocks(f, blocks, f->piece, f->x, f->hardDropY - f->fieldHidden, f->theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 2] = (TerminalCell) {
            .value = v->glyph.blockL,
            .attrs = ATTR_REVERSE | ATTR_DIM | attr_colour(f->piece)
        };
        v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 3] = (TerminalCell) {
            .value = v->glyph.blockR,
            .attrs = ATTR_REVERSE | ATTR_DIM | attr_colour(f->piece)
        };
    }

    ///
    // Current piece
    fsGetBlocks(f, blocks, f->piece, f->x, f->y - f->fieldHidden, f->theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 2] = (TerminalCell) {
            .value = v->glyph.blockL,
            .attrs = ATTR_REVERSE | attr_colour(f->piece)
        };
        v->bbuf[FIELD_Y + blocks[i].y][FIELD_X + 2*blocks[i].x + 3] = (TerminalCell) {
            .value = v->glyph.blockR,
            .attrs = ATTR_REVERSE | attr_colour(f->piece)
        };
    }
}

///
// Draw preview piece.
//
// Notes:
//  * Extend the maximum preview count beyond four in some way.
static void drawPreview(FSFrontend *v)
{
    i8x2 blocks[FS_NBP];
    const FSEngine *f = v->view->game;
    const int previewCount = f->nextPieceCount > FS_MAX_PREVIEW_COUNT
                                ? FS_MAX_PREVIEW_COUNT
                                : f->nextPieceCount;

    for (int i = 0; i < previewCount; ++i) {
        fsGetBlocks(f, blocks, f->nextPiece[i], 0, 0, 0);
        const int xpo = f->nextPiece[i] == FS_I || f->nextPiece[i] == FS_O ? 0 : 1;

        for (int j = 0; j < FS_NBP; ++j) {
            const int xo = PVIEW_X + xpo + 2 * blocks[j].x;
            const int yo = PVIEW_Y + (4 * i) + blocks[j].y;

            v->bbuf[yo][xo] = (TerminalCell) {
                .value = v->glyph.blockL,
                .attrs = ATTR_REVERSE | attr_colour(f->nextPiece[i])
            };
            v->bbuf[yo][xo + 1] = (TerminalCell) {
                .value = v->glyph.blockR,
                .attrs = ATTR_REVERSE | attr_colour(f->nextPiece[i])
            };
        }
    }
}

///
// Copy a string into the backbuffer at the specified coordinates.
//
// If this extends beyond the maximum terminal width then the string will be
// clipped to fit.
static void putStrAt(FSFrontend *v, const char *s, int y, int x, int attrs)
{
    if (y < 0 || FS_TERM_HEIGHT <= y || x < 0 || FS_TERM_WIDTH <= x) {
        return;
    }

    const int slen = strlen(s);
    for (int i = 0; i < slen; ++i) {
        if (x + i > FS_TERM_WIDTH) {
            break;
        }

        v->bbuf[y][x + i] = (TerminalCell) {
            .value = s[i],
            .attrs = attrs
        };
    }
}

///
// Render a string onto the middle of the field.
//
// The string will be centered, and truncated if too long.
void fsiRenderFieldString(FSFrontend *v, const char *msg)
{
    const FSEngine *f = v->view->game;
    const int w = strlen(msg);
    putStrAt(v, msg, FIELD_Y + FIELD_H / 2, FIELD_X + FIELD_W / 2 - w / 2 + 1, 0);
}

static void drawInfo(FSFrontend *v)
{
    const FSEngine *f = v->view->game;

    const int bufsiz = FS_TERM_WIDTH;
    char buf[bufsiz];

    // Target Goal is special and is drawn under the field.
    int remaining = v->view->game->goal - v->view->game->linesCleared;
    if (remaining < 0) {
        remaining = 0;
    }

    snprintf(buf, bufsiz, "%d", remaining);
    putStrAt(v, buf, FIELD_Y + FIELD_H + 1,
                FIELD_X + FIELD_W / 2 - strlen(buf) / 2 + 1, ATTR_BRIGHT);

    const int msElapsed = f->msPerTick * f->totalTicks;


    // Remaining items are drawn on the right-side of the field.
    snprintf(buf, bufsiz, "Time");
    putStrAt(v, buf, INFO_Y + 1, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%.3f", (float) msElapsed / 1000);
    putStrAt(v, buf, INFO_Y + 2, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "Blocks");
    putStrAt(v, buf, INFO_Y + 4, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%d", f->blocksPlaced);
    putStrAt(v, buf, INFO_Y + 5, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "TPS");
    putStrAt(v, buf, INFO_Y + 7, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%.5f",
             msElapsed != 0
                ? (float) f->blocksPlaced / ((float) msElapsed / 1000)
                : 0);
    putStrAt(v, buf, INFO_Y + 8, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "KPT");
    putStrAt(v, buf, INFO_Y + 10, INFO_X, ATTR_UNDERLINE);

    snprintf(buf, bufsiz, "%.5f",
             f->blocksPlaced ? (float) f->totalKeysPressed / f->blocksPlaced
                             : 0.0f);
    putStrAt(v, buf, INFO_Y + 11, INFO_X, ATTR_BRIGHT);

    snprintf(buf, bufsiz, "Faults");
    putStrAt(v, buf, INFO_Y + 13, INFO_X, ATTR_UNDERLINE);
    ;
    snprintf(buf, bufsiz, "%d", f->finesse);
    putStrAt(v, buf, INFO_Y + 14, INFO_X, ATTR_BRIGHT);
}

// This prints a single unicode code-point stored as a utf-8 byte array
// packed into a 32-bit value.
//
// Values are stored in reverse order, with the first first meaningful byte
// (representing the length) stored in bits 0-8.
static void put_single_utf8(uint32_t cp)
{
    const uint8_t bits[4] = {
        (cp >>  0) & 0xff,
        (cp >>  8) & 0xff,
        (cp >> 16) & 0xff,
        (cp >> 24) & 0xff,
    };

    if ((bits[0] & 0x80) == 0x00) {
        putchar(bits[0]);
    }
    else if ((bits[0] & 0xe0) == 0xc0) {
        putchar(bits[0]);
        putchar(bits[1]);
    }
    else if ((bits[0] & 0xf0) == 0xe0) {
        putchar(bits[0]);
        putchar(bits[1]);
        putchar(bits[2]);
    }
    else if ((bits[0] & 0xf8) == 0xf0) {
        putchar(bits[0]);
        putchar(bits[1]);
        putchar(bits[2]);
        putchar(bits[3]);
    }
    else {
        fsLogFatal("invalid utf8 codepoint encountered!");
    }
}

///
// Perform the actual draw for any pending operations.
//
// This uses a double-buffer system and will only draw segments of the screen
// which changed since the last draw.
//
// A complete redraw is performed if the invalidateBuffers flag is set. This
// will be unset after the function completes.
void fsiBlit(FSFrontend *v)
{
    // This seems pointless (and is!) but may be slightly tweaked in future.
    if (caughtSigwinch) {
        v->invalidateBuffers = true;
        caughtSigwinch = 0;
    }

    // Clear the entire screen on redraw
    if (v->invalidateBuffers) {
        printf("\033[H\033[2J");
    }

    // We have to perform some minor optimizations so that a complete redraw
    // can consistently occur within the draw limit.
    int lx = -1, ly = -1;
    for (int y = 0; y < FS_TERM_HEIGHT; ++y) {
        for (int x = 0; x < FS_TERM_WIDTH; ++x) {
            if (v->invalidateBuffers
                    || v->bbuf[y][x].value != v->fbuf[y][x].value
                    || v->bbuf[y][x].attrs != v->fbuf[y][x].attrs) {

                // Only update the attribute set if it is non-zero (default).
                bool attr_set = false;
                if (v->bbuf[y][x].attrs) {
                    for (int i = 0; i < ATTR_COUNT; ++i) {
                        if (v->bbuf[y][x].attrs & (1 << i)) {
                            printf("\033[%dm", attributes[i]);
                            attr_set = true;
                        }
                    }
                }

                if (x != lx + 1 || y != ly) {
                    printf("\033[%d;%dH", y + 1, x + 1);
                }

                lx = x;
                ly = y;

                put_single_utf8(v->bbuf[y][x].value);

                // Only reset attributes if they were altered.
                if (attr_set) {
                    printf("\033[0m");
                }
            }

            v->fbuf[y][x] = v->bbuf[y][x];
        }
    }

    // Always explcitly flush since we never print any newlines.
    fflush(stdout);
    v->invalidateBuffers = false;
}

///
// Add a trigger for the physical key from this virtual key.
void fsiAddToKeymap(FSFrontend *v, int virtualKey, const char *keyValue, bool isDefault)
{
    const int kc = fsKeyToPhysicalKey(keyValue);
    if (kc) {
        for (int i = 0; i < FS_MAX_KEYS_PER_ACTION; ++i) {
            KeyEntry *vk = &v->keymap[virtualKey][i];
            if (vk->value == KEY_NONE || vk->isDefault) {
                *vk = (KeyEntry) {
                    .value = kc,
                    .isDefault = isDefault
                };
                return;
            }
        }

        fsLogWarning("Could not insert key %s into full keymap", keyValue);
    }
}

///
// Called by the internal ini parser if a frontend option is encountered.
//
// Notes:
//  * There is quite a bit of redirection with this interface and would be nice
//    if we could adjust this slightly.
//
//  * This should be placed in a seperate file for better containment. (along
//    with future command line parsing).
void fsiUnpackFrontendOption(FSFrontend *v, const char *key, const char *value)
{
    if (!strcmpi(key, "glyphs")) {
        if (!strcmpi(value, "ascii")) {
            v->glyph = asciiGlyphSet;
        }
        else if (!strcmpi(value, "unicode")) {
            v->glyph = unicodeGlyphSet;
        }
        else {
            fsLogWarning("Ignoring unknown value %s for key %s", value, key);
        }

        return;
    }

    fsLogWarning("No suitable key found for option %s = %s", key, value);
}

///
// Perform a complete render -> blit loop.
void fsiDraw(FSFrontend *v)
{
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

///
// Run before every tick.
//
// Signal flags are handled here and not within the handlers themselves to
// avoid any unforeseen behaviour.
void fsiPreFrameHook(FSFrontend *v)
{
    (void) v;

    // If we encountered a SIGINT, then we want to reset the screen before we
    // re-raise it so the user can see the game content on exit.
    if (caughtSigint) {
        printf("\033[?25h");
        printf("\033[%d;%dH", FS_TERM_HEIGHT, FS_TERM_WIDTH);
        fflush(stdout);
        raise(SIGINT);
    }
}

///
// Run after every tick.
void fsiPostFrameHook(FSFrontend *v)
{
    (void) v;
}
