///
// main.c
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

#include "frontend.h"

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
    FSInput in = {0, 0, 0, 0, 0, 0};

    // We still want to handle quit and restart in a replay
    u32 keystate = fsiReadKeys(v);

    if (!g->replayPlayback) {
        fsReplayInsert(g->replay, f->totalTicksRaw, keystate);
    }
    else {
        //keystate &= FST_VK_FLAG_RESTART | FST_VK_FLAG_QUIT;
        keystate = fsReplayGet(g->replay, f->totalTicksRaw);
    }

    fsVirtualKeysToInput(&in, keystate, f, ctl);
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

    // Length of an average frame
    i32 avgFrame = 0;

    // The game loop here uses a fixed timestep with lag reduction. The render
    // phase is synced and occurs every `ticksPerDraw` frames.
    //
    // NOTE: This loop does not account for running too slow. We always assume
    // we can perform a `logic` -> `render` cycle within `tickRate`.
    while (1) {
        i32 startTime = fsiGetTime(v);
        i32 elapsed = startTime - lastTime;
        lastTime = startTime;

        // Lag can potentially be negative and would result in a slightly
        // longer frame being processed. This seems ok in practice, but we
        // could probably enforce this as non-negative if we tried.
        lag += (elapsed - tickRate);

        fsiPreFrameHook(v);
        updateGameLogic(v, g);

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
        avgFrame = avgFrame + ((currentTime - startTime) - avgFrame) / f->totalTicksRaw;

        // Break early if we know we are finished to save `tickRate` us of lag.
        if (lastFrame) {
            break;
        }

        // When should the tick end (best-case)
        const i32 tickEnd = startTime + tickRate;

        // Warn if a frame was too slow but don't do anything special.
        if (tickEnd < currentTime) {
            fsLogDebug("Tick %ld took %ld but tickrate is only %ld",
                        f->totalTicks, currentTime - startTime, tickRate);
        }

        // If frame has taken too long to render then this could be negative.
        // Avoid the underflow (resulting in a long sleep).
        const i32 value = tickEnd - lag - currentTime;
        fsiSleepUs(v, value > 0 ? value : 0);
    }

    // Cross-reference the in-game time (as calculated from the number of
    // elapsed ticks) to a reference clock to ensure it runs accurately.
    const double actualElapsed = (double) f->actualTime / 1000000;
    const double ingameElapsed = (double) (f->totalTicksRaw * f->msPerTick) / 1000;

    fsLogDebug("Average frame time: %d", avgFrame);
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
                // We must reinit the replay buffer every restart.
                // TODO: Don't keep files open!
                // TODO: Fix annoying seed set here. Unintuitive.
                if (!g->replayPlayback) {
                    g->game->seed = fsGetRoughSeed();
                    fsReplayInit(g->game, g->replay);
                }
                else {
                    fsReplayLoad(g->game, g->replay);
                }

                fsGameReset(g->game);
                playGameLoop(v, g);

                switch (g->game->state) {
                    case FSS_RESTART:
                        fsReplayClear(g->replay);
                        g->replayPlayback = false;
                        // Stay in current state to restart
                        goto start;

                    case FSS_QUIT:
                        g->replayPlayback = false;
                        goto end;

                    case FSS_GAMEOVER:
                        if (!g->replayPlayback) {
                            fsReplaySave(g->replay);
                        }

                        g->replayPlayback = false;
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
    FSReplay replay;
    FSView gView = { .game = &game, .control = &control, .replay = &replay,
                     .replayPlayback = false, .totalFramesDrawn = 0 };
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

    if (o.replay) {
        gView.replayPlayback = true;
    }

    fsiInit(&pView);
    gameLoop(&pView, &gView);

    fsiFree(&pView);

#ifdef FS_USE_TERMINAL
    fclose(fsLogStream);
#endif
}
