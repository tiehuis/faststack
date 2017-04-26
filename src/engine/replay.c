///
// replay.c
// ========
//
// Manages loading and saving of replay files.
///

#include "engine.h"
#include "replay.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#define REPLAY_TMPDIR "replay/"
#define REPLAY_TMPFILE ".current"

// We ignore invalid boolean reads. A boolean must be at least 8 bits on
// any reasonable platform so this should be fine for the most part.
#pragma GCC diagnostic ignored "-Wformat"

#define pi8(id) fprintf(r->handle, "%"PRId8"\n", f->id)
#define pi32(id) fprintf(r->handle, "%"PRId32"\n", f->id)

void serializeOptions(const struct FSEngine *f, FSReplay *r)
{
    pi8(fieldWidth);
    pi8(fieldHeight);
    pi8(fieldHidden);
    pi8(initialActionStyle);
    pi8(dasSpeed);
    pi32(dasDelay);
    pi8(msPerTick);
    pi32(ticksPerDraw);
    pi32(areDelay);
    pi8(areCancellable);
    pi8(lockStyle);
    pi32(lockDelay);
    pi8(floorkickLimit);
    pi8(oneShotSoftDrop);
    pi8(rotationSystem);
    pi32(gravity);
    pi32(softDropGravity);
    pi8(randomizer);
    pi32(readyPhaseLength);
    pi32(goPhaseLength);
    pi8(infiniteReadyGoHold);
    pi8(nextPieceCount);
    pi32(goal);
    fprintf(r->handle, "%"PRIu32"\n", f->seed);
    fprintf(r->handle, "\n");
}

#define C(count, expr)                                                          \
    do {                                                                        \
        int result = expr;                                                      \
        if (result != (count)) {                                                \
            fsLogWarning("%d, %d", result, count);\
            fsLogWarning("failed to deserialize replay file: %d", __LINE__);    \
            exit(1);                                                            \
        }                                                                       \
    } while (0);

#define si8(id) C(1, fscanf(r->handle, "%"SCNd8"\n", &f->id))
#define si32(id) C(1, fscanf(r->handle, "%"SCNd32"\n", &f->id))

void deserializeOptions(struct FSEngine *f, FSReplay *r)
{
    si8(fieldWidth);
    si8(fieldHeight);
    si8(fieldHidden);
    si8(initialActionStyle);
    si8(dasSpeed);
    si32(dasDelay);
    si8(msPerTick);
    si32(ticksPerDraw);
    si32(areDelay);
    si8(areCancellable);
    si8(lockStyle);
    si32(lockDelay);
    si8(floorkickLimit);
    si8(oneShotSoftDrop);
    si8(rotationSystem);
    si32(gravity);
    si32(softDropGravity);
    si8(randomizer);
    si32(readyPhaseLength);
    si32(goPhaseLength);
    si8(infiniteReadyGoHold);
    si8(nextPieceCount);
    si32(goal);
    C(1, fscanf(r->handle, "%"SCNu32"\n", &f->seed));
    C(0, fscanf(r->handle, "\n"));
}

// Create a new filename for storing the current replay.
char* replayFileName(const struct FSEngine *f)
{
    static char buffer[64];
    time_t rawtime;
    struct tm *tminfo;

    time(&rawtime);
    tminfo = localtime(&rawtime);
    snprintf(buffer, 64, "%d'%.3f'", f->goal, (float) (f->msPerTick * f->totalTicks) / 1000);
    strftime(buffer + strlen(buffer), 64 - strlen(buffer), "%F'%H:%M:%S", tminfo);

    return buffer;
}

/// Initialize a replay file.
///
/// This requires an `FSEngine` in order to save the option state for
/// accurate playback.
void fsReplayInit(const struct FSEngine *f, FSReplay *r)
{
    r->error = false;
    if (r->error) {
        // We failed once, so we probably would fail again.
        // Avoid warning multiple times on restarts.
        return;
    }

    // TODO: Allow this to be resolved at runtime, adding a `local` flag to
    // not use external directories.
    r->handle = fopen(REPLAY_TMPDIR REPLAY_TMPFILE, "w+");
    if (r->handle != NULL) {
        r->lastKeystate = 0;
        serializeOptions(f, r);
    }
    else {
        r->error = true;
        fsLogWarning("failed to create replay file");
    }
}

/// Add an entry to the current replay file.
void fsReplayInsert(FSReplay *r, u32 ticks, u32 keystate)
{
    if (!r->error && keystate != r->lastKeystate) {
        fprintf(r->handle, "%"PRId32",%"PRIx32"\n", ticks, keystate);
        r->lastKeystate = keystate;
    }
}

/// Finished writing all data out to the replay.
///
/// Cleans up any open resources as well. `fsReplayInit` can be called
/// again following this.
void fsReplaySave(const struct FSEngine *f, FSReplay *r)
{
    if (!r->error) {
        fflush(r->handle);
        fclose(r->handle);

        char target[64];
        strcpy(target, REPLAY_TMPDIR);
        strcat(target, replayFileName(f));

        // Rename the current file with the current time
        rename(REPLAY_TMPDIR REPLAY_TMPFILE, target);
    }
}

/// Load a replay file for reading. The next key is queried with
/// `fsReplayGet`.
void fsReplayLoad(struct FSEngine *f, FSReplay *r, const char *filename)
{
    r->error = false;
    r->handle = fopen(filename, "r");
    if (r->handle != NULL) {
        deserializeOptions(f, r);
        r->lastKeystate = 0;
        if (fscanf(r->handle, "%"SCNd32",%"SCNx32"\n", &r->currentTick, &r->currentKeystate) != 2) {
            // We have no input, discard error
        }
    }
    else {
        printf("failed to load replay file: %s\n", filename);
        exit(1);
    }
}

/// Get the keystate for the given tick.
u32 fsReplayGet(FSReplay *r, u32 ticks)
{
    if (ticks == r->currentTick) {
        r->lastKeystate = r->currentKeystate;
        if (fscanf(r->handle, "%"SCNd32",%"SCNx32"\n", &r->currentTick, &r->currentKeystate) != 2) {
            // We are at the end of the file, discard error
        }
    }

    return r->lastKeystate;
}

/// Clear the replay handle, closing the file.
///
/// This cleans up resources without storing the replay and is intended
/// for early exit functions.
void fsReplayClear(FSReplay *r)
{
    if (!r->error) {
        fclose(r->handle);
        remove(REPLAY_TMPDIR REPLAY_TMPFILE);
    }
}
