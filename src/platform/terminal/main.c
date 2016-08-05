// FastStack-Term
//
// Terminal frontend to the FastStack engine.
//
// Since we are using VT100 escape codes, we perform a lot of internal
// buffering implicitly via the FSView object.
// We use the flag to allow this support at compile time, since it
// requires a little more work to do otherwise.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/input.h>

#include "../../fs.h"
#include "../../fsPlay.h"

// This is the input device on my machine. This could differ on other machines and a
// way to determine this generically would be nice.
static const char *inputDeviceName = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";

// Hold Segment
#define HD_WIDTH  10 // 2 (border) + 4 (2 wide blocks)
#define HD_HEIGHT 8  // 2 (border) + 3 (1 high blocks)

// Preview Segment
#define PV_WIDTH  HD_WIDTH
#define PV_HEIGHT (FS_PREVIEW_MAX * HD_HEIGHT)

// Field Segment
#define FD_WIDTH  (2 + 2 * FS_MAX_WIDTH) // 2 (border) + FW (2 wide)
#define FD_HEIGHT (2 + FS_MAX_HEIGHT)    // 2 (border) + FH (1 high)

// Platform-Specific view
struct FSPSView {
    // The generic backing view
    FSView *view;

    // File descriptor of the currently open input device
    int inputFd;

    // Initial terminal state
    struct termios initialTermState;

    // Indicate whether the buffer state is valid or not
    bool invalidBuffers;

    // Hold Segment buffers
    char hdbbuf[HD_HEIGHT][HD_WIDTH];
    char hdfbuf[HD_HEIGHT][HD_WIDTH];

    // Preview Segment buffers
    char pvbbuf[PV_HEIGHT][PV_WIDTH];
    char pvfbuf[PV_HEIGHT][PV_WIDTH];

    // Field Segment buffers
    char fdbbuf[PV_HEIGHT][PV_WIDTH];
    char fdfbuf[PV_HEIGHT][PV_WIDTH];
};

// Initialize this terminal instance and input devices
static void initTerm(FSPSView *v)
{
    v->inputFd = open(inputDeviceName, O_RDONLY);

    if (v->inputFd == -1) {
        if (errno == EACCES) {
            fprintf(stderr,
                    "Insufficient permission to open device: %s\n"
                    "Check the README for details\n",
                    inputDeviceName);
        }
        else {
            perror("Error opening device:");
        }

        exit(1);
    }

    // Hide the cursor
    printf("\e[?25l");
    fflush(stdout);

    // Indicate that the buffer data is invalid and we should redraw everything
    v->invalidBuffers = true;

    // Set terminal properly with termios
    tcgetattr(STDIN_FILENO, &v->initialTermState);

    struct termios newTermState = v->initialTermState;
    newTermState.c_lflag &= ~(ECHO | ICANON);
    newTermState.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermState);
}

static void destroyTerm(FSPSView *v)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &v->initialTermState);

    // Restore cursor
    printf("\e[?25h");
    fflush(stdout);

    if (close(v->inputFd) == -1) {
        perror("Error closing device:");
        exit(2);
    }
}

// Return the clock time in microsecond granularity.
FSLong fsGetTime(FSPSView *v)
{
    (void) v;
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// Sleep for the specified number of microseconds.
void fsSleepUs(FSPSView *v, FSLong time)
{
    (void) v;
    usleep(time);
}

// Return if there are bytes pending in the output queue
static bool inputPending(void)
{
    int bytesPending;
    ioctl(STDOUT_FILENO, FIONREAD, &bytesPending);
    return bytesPending;
}

// Return the set of virtual keys that were read from the physical device.
// This should also handle any other events that aren't explicitly keys.
FSBits fsReadKeys(FSPSView *v)
{
    char keyState[(KEY_MAX + 7) / 8] = {0};

    // We should put this in fscontrol.h
    static const FSBits virtualKeys[] = {
        VKEY_UP, VKEY_DOWN, VKEY_LEFT, VKEY_RIGHT, VKEY_ROTL,
        VKEY_ROTR, VKEY_ROTH, VKEY_HOLD, VKEY_START
    };

    static const int physicalKeys[] = {
        KEY_SPACE, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_Z,
        KEY_X, KEY_A, KEY_C, KEY_KPENTER
    };

    // Characters are still being printed, but are just not being displayed.
    // Periodically empty the output queue so it doesn't grow too large.
    while (inputPending()) {
        getchar();
    }

    // Read physical keyboard state into buffer
    ioctl(v->inputFd, EVIOCGKEY(sizeof(keyState)), keyState);

    FSBits keys = 0;
    for (int i = 0; i < 9; ++i) {
        // Extract the target key into a queryable structure
        const int key = physicalKeys[i];
        const int keyQuery = keyState[key >> 3];
        const int keyMask  = 1 << (key & 7);

        if (keyQuery & keyMask) {
            keys |= virtualKeys[i];
        }
    }

    return keys;
}

static void drawField(FSPSView *v)
{
    const FSGame *f = v->view->game;

    // Insert field to backbuffer
    for (int y = 0; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            v->fdbbuf[y][x] = f->b[y][x] ? '#' : ' ';
        }
    }

    // Insert current piece to backbuffer and ghost
    const FSBlock pid = v->view->game->piece;

    FSInt2 blocks[4];
    fsPieceToBlocks(f, blocks, pid, f->x, f->hardDropY, f->theta);

    // Offset x by 1 for first border
    for (int i = 0; i < FS_NBP; ++i) {
        const int xo = blocks[i].x;
        const int yo = blocks[i].y;

        v->fdbbuf[yo][xo] = '@';
        v->fdbbuf[yo][xo] = '@';
    }

    fsPieceToBlocks(f, blocks, pid, f->x, f->y, f->theta);

    // Offset x by 1 for first border
    for (int i = 0; i < FS_NBP; ++i) {
        const int xo = blocks[i].x;
        const int yo = blocks[i].y;

        v->fdbbuf[yo][xo] = '&';
        v->fdbbuf[yo][xo] = '&';
    }
}


static void blitBackbuffer(FSPSView *v)
{
    const FSGame *f = v->view->game;

    // If buffers are invalid clear the entire screen and restart
    if (v->invalidBuffers)
        printf("\e[H\e[2J");

    for (int y = 0; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            if (v->invalidBuffers || v->fdbbuf[y][x] != v->fdfbuf[y][x]) {
                printf("\e[%d;%dH%c", y + 1, x + 1, v->fdbbuf[y][x]);
            }
            v->fdfbuf[y][x] = v->fdbbuf[y][x];
        }
    }

    // Flush buffers since we wont push a newline at all
    fflush(stdout);

    if (v->invalidBuffers)
        v->invalidBuffers = false;
}

// Peform an entire draw, blit loop
void fsDraw(FSPSView *v)
{
    drawField(v);
    blitBackbuffer(v);
}

// Run before any user code is processed in a tick
void fsPreFrameHook(FSPSView *v)
{
    (void) v;
}

// Run after we have slept for the specified period of time
void fsPostFrameHook(FSPSView *v)
{
    (void) v;
}

int main(void)
{
    FSGame game = {
        .fieldWidth = 10,
        .fieldHeight = 20,
        .msPerTick = 8
    };

    FSControl control = {
        .dasSpeed = 0,
        .dasDelay = 150
    };

    FSView genericView = {
        .game = &game,
        .control = &control,
        .totalFramesDrawn = 0
    };

    FSPSView mainView = {
        .view = &genericView,
    };

    initTerm(&mainView);

    fsGameClear(mainView.view->game);
    fsParseIniFile(mainView.view, "fs.ini");

    // Draw a rudimentary menu waiting for any keypress
    // Start the game - we need to pass the generic view since we do not know
    // about the internal structure.
    fsPlayStart(&mainView, mainView.view);

    destroyTerm(&mainView);
}
