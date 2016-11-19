///
// replay.h
// ========
//
// Support for loading/reading replay files.
///

#ifndef FS_REPLAY_H
#define FS_REPLAY_H

#include "core.h"

#ifndef FS_DISABLE_REPLAY

#include <stdio.h>

// A generic loader/reader of a replay file.
struct FSReplay {
    // File to read/write date to/from.
    FILE *handle;

    // Name of the backing file.
    char fname[64];

    // Last known keystate. Used for computing deltas.
    u32 lastKeystate;

    // Current tick value read.
    u32 currentTick;

    // Current keystate read.
    u32 currentKeystate;

    // Error flag preserved over re-initialization.
    bool error;
};


// Initializes a replay as a hidden file within the replay directory.
//
// This is considered temporary until either one of `fsReplaySave` or
// `fsReplayClear` is called.
void fsReplayInit(const FSEngine *f, FSReplay *r);

// Try and insert the current keystate value into the replay file.
//
// The value `ticks` is expected to be called with monotonically increasing
// values. If the `keystate` does not differ from the internally cached one
// nothing will be written.
void fsReplayInsert(FSReplay *r, u32 ticks, u32 keystate);

// Finalize a replay files contents, writing any remaining contents to disk.
//
// This will rename the working replay file to one based on the current date,
// goal, and resulting performance (playtime).
void fsReplaySave(const FSEngine *f, FSReplay *r);

// Load an existing replay file from the specified filename.
//
// This will emit a warning if loading failed, and will set the internal
// `error` flag to true which will set any following calls to replay functions
// as effective noops.
void fsReplayLoad(FSEngine *f, FSReplay *r, const char *filename);

// Read the expected keystate from the replay file for the given `ticks` value.
//
// `ticks` values are expected to be called in a monotonically increasing
// fashion and will not work as a random access means.
u32 fsReplayGet(FSReplay *r, u32 ticks);

// Finalize any resources used by the replay instance.
//
// This closes any open file handles.
void fsReplayClear(FSReplay *r);

#else

// Struct must have at least one member in standard C
struct FSReplay { char _; };

#define fsReplayInit(f, r)
#define fsReplayInsert(r, ticks, keystate)
#define fsReplaySave(f, r)
#define fsReplayLoad(f, r, filename)
#define fsReplayGet(r, ticks) 0
#define fsReplayClear(r)

#endif // FS_DISABLE_REPLAY

#endif
