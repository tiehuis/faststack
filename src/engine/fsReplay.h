///
// fsReplay.h
// ==========
//
// Defines the structure for working with replay files.
///

#ifndef FS_REPLAY_H
#define FS_REPLAY_H

#include "fsCore.h"
#include "fsConfig.h"
#include <stdbool.h>
#include <stdio.h>

struct FSEngine;

typedef struct {
    // File to write replay data out too
    FILE *handle;

    // Name of file
    char fname[64];

    // Last known keystate
    u32 lastKeystate;

    // Current tick value
    u32 currentTick;

    // Current keystate
    u32 currentKeystate;

    // Explicit error flag preserved over init
    bool error;
} FSReplay;

void fsReplayInit(const struct FSEngine *f, FSReplay *r);
void fsReplayInsert(FSReplay *r, u32 ticks, u32 keystate);
void fsReplaySave(const struct FSEngine *f, FSReplay *r);
void fsReplayFree(FSReplay *r);
void fsReplayLoad(struct FSEngine *f, FSReplay *r, const char *filename);
u32 fsReplayGet(FSReplay *r, u32 ticks);
void fsReplayClear(FSReplay *r);

#endif
