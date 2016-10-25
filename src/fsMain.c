///
// fsMain.c
// ========
//
// Main entry point. This will run the program and call any required interface
// functions.
///

// Need to include the actual frontend we are using here so we know the storage
// size.
#include "fsEngine.h"
#include "fsDefault.h"
#include "fsInterface.h"
#include "fsOption.h"
#include "fsFrontend.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void fsLoadDefaultKeys(FSPSView *v)
{
#define ADD_KEY(name) fsiAddToKeymap(v, FST_VK_##name, FSD_KEY_##name, true)

    ADD_KEY(UP);
    ADD_KEY(DOWN);
    ADD_KEY(LEFT);
    ADD_KEY(RIGHT);
    ADD_KEY(ROTL);
    ADD_KEY(ROTR);
    ADD_KEY(ROTH);
    ADD_KEY(HOLD);
    ADD_KEY(RESTART);
    ADD_KEY(QUIT);

#undef ADD_KEY
}

static void updateGameLogic(FSPSView *v, FSView *g)
{
    FSEngine *f = g->game;
    FSControl *ctl = g->control;
    FSInput in = {0, 0, 0, 0, 0};

    fsVirtualKeysToInput(&in, fsiReadKeys(v), f, ctl);
    fsGameTick(f, &in);
}

static void drawStateStrings(FSPSView *v, FSView *g)
{
    switch (g->game->state) {
        case FSS_READY:
            fsiRenderFieldString(v, "READY");
            break;
        case FSS_GO:
            fsiRenderFieldString(v, "GO");
            break;
        default:
            break;
    }
}

static void updateGameView(FSPSView *v, FSView *g)
{
    fsiDraw(v);
    drawStateStrings(v, g);
    fsiPlaySe(v, g->game->se);
    g->totalFramesDrawn += 1;
}

static void playGameLoop(FSPSView *v, FSView *g)
{
    FSEngine *f = g->game;
    i32 tickRate = f->msPerTick * 1000;
    i32 gameStart = fsiGetTime(v);
    i32 lastTime = fsiGetTime(v);
    i32 lag = 0;

    // The game loop here uses a fixed timestep with lag reduction. The render
    // phase is synced and occurs every `ticksPerDraw` frames.
    //
    // NOTE: This loop does not account for running too slow. We always assume
    // we can perform a `logic` -> `render` cycle within `tickRate`.
    while (1) {
        i32 startTime = fsiGetTime(v);
        i32 elapsed = startTime - lastTime;
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
                               f->state == FSS_RESTART ||
                               f->state == FSS_QUIT;

        // We always want to draw the final frame, even if we were in between
        // ticks.
        if (f->totalTicks % f->ticksPerDraw == 0 || lastFrame) {
            updateGameView(v, g);
            fsiPostFrameHook(v);
            fsiBlit(v);
        }

        i32 currentTime = fsiGetTime(v);
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

// As close to a menu as we'll get.
void gameLoop(FSPSView *v, FSView *g)
{
    enum {
        IN_GAME, IN_EXCELLENT, IN_WAIT
    };

    int state = IN_GAME;
    int counter = 0;

    while (1) {
start:;
        const u32 keys = fsiReadKeys(v);

        // Allow a reset or restart from anywhere (this is managed by the
        // frame hooks during an actual game).
        if (keys & FST_VK_FLAG_RESTART) {
            state = IN_GAME;
        }
        if (keys & FST_VK_FLAG_QUIT) {
            break;
        }

        switch (state) {
            case IN_GAME:
            {
                fsGameReset(g->game);
                playGameLoop(v, g);

                switch (g->game->state) {
                    case FSS_RESTART:
                        // Stay in current state to restart
                        goto start;

                    case FSS_QUIT:
                        goto end;

                    case FSS_GAMEOVER:
                        state = IN_EXCELLENT;
                        counter = 0;
                        break;

                    default:
                        fsLogError("Encountered unknown state");
                        exit(2);
                }
            }

            case IN_EXCELLENT:
            {
                // Use an explicit draw here to ensure strings don't overwrite
                // one another.
                fsiDraw(v);
                fsiRenderFieldString(v, "EXCELLENT");
                if (counter >= 125) {
                    state = IN_WAIT;
                    counter = 0;
                }
                break;
            }

            case IN_WAIT:
            {
                fsiDraw(v);
                fsiRenderFieldString(v, "RSHIFT TO PLAY AGAIN");
                break;
            }

            default:
            {
                fsLogError("Encountered invalid state", state);
                exit(2);
            }
        }

        fsiBlit(v);
        fsiSleepUs(v, 16 * 1000);
        counter++;
    }
end:;
}

int main(int argc, char **argv)
{
    FSEngine game;
    FSControl control;
    FSView gView = { .game = &game, .control = &control, .totalFramesDrawn = 0 };
    FSPSView pView = { .view = &gView };

    FSOptions o;
    fsParseOptString(&o, argc, argv);

#ifdef FS_USE_TERMINAL
    fsLogStream = fopen(FS_LOG_FILENAME, "w+");
#else
    fsLogStream = stderr;
#endif

    if (o.verbosity) {
        fsCurrentLogLevel = o.verbosity;
    }

    fsiPreInit(&pView);
    fsGameInit(&game);
    fsLoadDefaultKeys(&pView);

    if (!o.no_ini) {
        fsParseIniFile(&pView, &gView, FS_CONFIG_FILENAME);
    }

    fsiInit(&pView);
    gameLoop(&pView, &gView);

    fsiFree(&pView);

#ifdef FS_USE_TERMINAL
    fclose(fsLogStream);
#endif
}
