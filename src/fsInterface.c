///
// fsInterface.c
//
// Main game loop and interface between frontend and engine code.
//
// All functions prefixed with fsi* **must** be implemented by a
// frontend.
//
// Notes:
//  - Test dropping tick input instead of overclocking. Potentially
//    allow as an option?
///

#include <stdio.h>
#include <math.h>
#include "fs.h"
#include "fsInterface.h"

static void updateGameLogic(FSPSView *v, FSView *g)
{
    FSGame *f = g->game;
    FSControl *ctl = g->control;
    FSInput in = {0, 0, 0, 0};

    fsVirtualKeysToInput(&in, fsiReadKeys(v), f, ctl);
    fsGameTick(f, &in);
}

static void updateGameView(FSPSView *v, FSView *g)
{
    fsiDraw(v);
    fsiPlaySe(v, g->game->se);

    // Acknowledge that we played the sound effects
    g->totalFramesDrawn += 1;
}

void fsGameLoop(FSPSView *v, FSView *g)
{
    FSGame *f = g->game;
    FSLong tickRate = f->msPerTick * 1000;
    FSLong gameStart = fsiGetTime(v);
    FSLong lastTime = fsiGetTime(v);
    FSLong lag = 0;

    // The game loop here uses a fixed timestep approach with lag reduction
    // in case we accumulate to much extra time.
    //
    // We do not account at all for the game running too slow. We should always
    // be able to perform a logic->render cycle in the specified time.
    while (1) {
        FSLong startTime = fsiGetTime(v);
        FSLong elapsed = startTime - lastTime;
        lastTime = startTime;
        lag += elapsed;

        fsiPreFrameHook(v);

        // If we accumulate more than tickRate lag, then perform an
        // extra frame of input. This will reuse the same input so some
        // odd things could potentially happen here that may not be
        // optimal.
        //
        // Another option to test is whether just that we made up the tick
        // is viable. This would have the effect of ensuring we don't get
        // repeated inputs, but at the cost of potential lost inputs.
        {
            updateGameLogic(v, g);
            lag -= tickRate;
        }
        if (lag >= tickRate) {
            updateGameLogic(v, g);
            lag -= tickRate;
        }

        updateGameView(v, g);
        fsiPostFrameHook(v);

        // Blit after postFrameHook to allow final rendering
        fsiBlit(v);

        // Update actual game time
        FSLong currentTime = fsiGetTime(v);
        f->actualTime = currentTime - gameStart;

        // If we received an end event, finish before sleeping. This
        // saves one tick of lag (minor).
        if (f->state == FSS_GAMEOVER || f->state == FSS_QUIT)
            break;

        fsiSleepUs(v, startTime + tickRate - currentTime);
    }

    // Check if the game ran at an appropriate rate.
    const double actualElapsed = (double) f->actualTime / 1000000;
    const double ingameElapsed = (double) (f->totalTicks * f->msPerTick) / 1000;

    fsLogDebug("Actual time elapsed: %lf", actualElapsed);
    fsLogDebug("Ingame time elapsed: %lf", ingameElapsed);
    fsLogDebug("Maximum Difference: %lf", fabs(actualElapsed - ingameElapsed));
}
