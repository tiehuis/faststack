///
// fsInterface.c
// =============
//
// Main game loop and interface between frontend and engine code.
//
// All functions prefixed with fsi* **must** be implemented by a
// frontend.
///

#include "fs.h"
#include "fsInterface.h"

#include <stdio.h>
#include <math.h>

static void updateGameLogic(FSPSView *v, FSView *g)
{
    FSGame *f = g->game;
    FSControl *ctl = g->control;
    FSInput in = {0, 0, 0, 0, 0};

    fsVirtualKeysToInput(&in, fsiReadKeys(v), f, ctl);
    fsGameTick(f, &in);
}

static void updateGameView(FSPSView *v, FSView *g)
{
    fsiDraw(v);
    fsiPlaySe(v, g->game->se);
    g->totalFramesDrawn += 1;
}

void fsGameLoop(FSPSView *v, FSView *g)
{
    FSGame *f = g->game;
    FSLong tickRate = f->msPerTick * 1000;
    FSLong gameStart = fsiGetTime(v);
    FSLong lastTime = fsiGetTime(v);
    FSLong lag = 0;

    // The game loop here uses a fixed timestep with lag reduction. The render
    // phase is synced and occurs every `ticksPerDraw` frames.
    //
    // NOTE: This loop does not account for running too slow. We always assume
    // we can perform a `logic` -> `render` cycle within `tickRate`.
    while (1) {
        FSLong startTime = fsiGetTime(v);
        FSLong elapsed = startTime - lastTime;
        lastTime = startTime;
        lag += elapsed;

        fsiPreFrameHook(v);

        // We need handle at most only one frame of lag at a time. This
        // is enough for correcting the clock sleep lag.
        //
        // NOTE: Test whether dropping input during lag is works as an
        // alternative to doubling input.
        {
            updateGameLogic(v, g);
            lag -= tickRate;
        }
        if (lag >= tickRate) {
            updateGameLogic(v, g);
            lag -= tickRate;
        }

        const bool lastFrame = f->state == FSS_GAMEOVER ||
                               f->state == FSS_QUIT;

        // We always want to draw the final frame, even if we were in between
        // ticks.
        if (f->totalTicks % f->ticksPerDraw == 0 || lastFrame) {
            updateGameView(v, g);
            fsiPostFrameHook(v);
            fsiBlit(v);
        }

        FSLong currentTime = fsiGetTime(v);
        f->actualTime = currentTime - gameStart;

        // Break early if we know we are finished to save `tickRate` us of lag.
        if (lastFrame) {
            break;
        }

        // NOTE: This should try to do more corrections if possible.
        if (startTime + tickRate < currentTime) {
            fsLogDebug("Tick %ld took %ld but tickrate is only %ld",
                        f->totalTicks, currentTime - startTime, tickRate);
        }
        else {
            fsiSleepUs(v, startTime + tickRate - currentTime);
        }
    }

    // Cross-reference the in-game time (as calculated from the number of
    // elapsed ticks) to a reference clock to ensure it runs accurately.
    const double actualElapsed = (double) f->actualTime / 1000000;
    const double ingameElapsed = (double) (f->totalTicks * f->msPerTick) / 1000;

    fsLogDebug("Actual time elapsed: %lf", actualElapsed);
    fsLogDebug("Ingame time elapsed: %lf", ingameElapsed);
    fsLogDebug("Maximum Difference: %lf", fabs(actualElapsed - ingameElapsed));
}
