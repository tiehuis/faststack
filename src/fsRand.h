///
// fsRand.h
// ========
//
// Provides functions which rely on random state.
///

#ifndef FS_RAND_H
#define FS_RAND_H

#include "fsTypes.h"

/// Randomizer type
enum RandomizerType {
    FST_RAND_UNDEFINED,
    FST_RAND_SIMPLE,
    FST_RAND_NOSZO_BAG7,
    FST_RAND_TGM1,
    FST_RAND_TGM2
};

struct FSGame;

///
// Random state context.
//
// This stores the current data used to compute the next random value. Based on
// the prng here: http://burtleburtle.net/bob/rand/smallprng.html.
///
typedef struct FSRandCtx {
    u32 a, b, c, d;
} FSRandCtx;

u32 fsGetRoughSeed(void);

u32 fsRandNext(FSRandCtx *ctx);

void fsRandSeed(FSRandCtx *ctx, u32 seed);

FSBlock fsNextRandomPiece(struct FSGame *f);

#endif // FS_RAND_H
