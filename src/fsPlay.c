// The main game loop.
//
// The functions provided by the interface in fsplay.h need to be
// implemented by the frontend.
//
// Variable timestep seems like a bad idea. Keep FPS a multiple of the
// logic rate so we always update incrementally in sync?
//
// Try a variable time-step though and see how it fares.
// Check Nullpomino and Lockjaw main loop since this runs slow it seems.

#include "fs.h"
#include "fsPlay.h"

#include <stdio.h>

static void updateGameLogic(FSPSView *v, FSView *g)
{
    // We assume only one player currently
    FSGame *const f = g->game;
    FSControl *const ctl = g->control;
    FSInput in = {0, 0, 0, 0};

    fsVirtualKeysToInput(&in, fsReadKeys(v), f, ctl);

    // Perform all game logic here
    fsGameDoTick(f, &in);
}

static void updateGameView(FSPSView *v, FSView *g)
{
    fsDraw(v);
    g->totalFramesDrawn += 1;
}

void fsPlayStart(FSPSView *v, FSView *g)
{
    const FSGame *f = g->game;
    const FSLong tickRate = f->msPerTick * 1000;
    FSLong gameStart = fsGetTime(v);
    FSLong lastTime = fsGetTime(v);
    FSLong lag = 0;

    // We always want to iterate over one loop at least
    while (1) {
        FSLong startTime = fsGetTime(v);
        FSLong elapsed = startTime - lastTime;
        lastTime = startTime;
        lag += elapsed;

        // Allow us to catch up against any lost time due to oversleep.
        // Only allow a maximum of one frame of lag to be made up per
        // tick. This should never really be exceeded and if so, a
        // gentle catch up is probably less noticeable than suddenly
        // performing multiple updates in succession.
        //
        // If multiple updates are repeatedly occuring then the game is
        // likely running horridly anyway.
        {
            updateGameLogic(v, g);
            lag -= tickRate;
        }
        if (lag >= tickRate) {
            updateGameLogic(v, g);
            lag -= tickRate;
        }

        updateGameView(v, g);

        // Break immediately on game over instead of sleeping
        if (f->state == FSS_GAMEOVER || f->state == FSS_QUIT)
            break;

        // Sleep for approximately the correct time
        fsSleepUs(v, startTime + tickRate - fsGetTime(v));
    }

    // This can now be used to confirm that the timing from totalTicks was
    // accurate for this game.
    g->game->actualTime = fsGetTime(v) - gameStart;
    printf("%lf\n", (double) g->game->actualTime / 1000000);
}
