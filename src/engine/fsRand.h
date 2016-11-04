///
// fsRand.h
// ========
//
// Provides functions which rely on random state.
///

#ifndef FS_RAND_H
#define FS_RAND_H

#include "fsCore.h"

enum RandomizerType {
    FST_RAND_UNDEFINED,
    FST_RAND_SIMPLE,
    FST_RAND_NOSZO_BAG7,
    FST_RAND_TGM1,
    FST_RAND_TGM2
};

struct FSEngine;

///
// Random state context.
//
// This stores the current data used to compute the next random value. Based on
// the prng here: http://burtleburtle.net/bob/rand/smallprng.html.
///
typedef struct FSRandCtx {
    u32 a, b, c, d;
} FSRandCtx;

/// Return a random seed value which updates frequently enough.
u32 fsGetRoughSeed(void);

/// Return the next u32 in the PRNG sequence
u32 fsRandNext(FSRandCtx *ctx);

/// Seed the random context
void fsRandSeed(FSRandCtx *ctx, u32 seed);

/// Update the engine with a new random piece
FSBlock fsNextRandomPiece(struct FSEngine *f);

#endif // FS_RAND_H
