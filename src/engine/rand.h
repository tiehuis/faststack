///
// rand.h
// ======
//
// Header file for all random function support.
//
// This exposes its own random function instead of utilizing those found in
// `stdlib.h`. The reason this is required is to ensure consistent random
// number generation across arbitrary platforms.
///

#ifndef FS_RAND_H
#define FS_RAND_H

#include "core.h"

enum RandomizerType {
    FST_RAND_UNDEFINED,
    FST_RAND_SIMPLE,
    FST_RAND_BAG7,
    FST_RAND_TGM1,
    FST_RAND_TGM2,
    FST_RAND_TGM3,
    FST_RAND_BAG7_SEAM_CHECK,
    FST_RAND_BAG6,
    FST_RAND_MULTI_BAG2,
    FST_RAND_MULTI_BAG4,
    FST_RAND_MULTI_BAG9
};

///
// Random state context.
//
// This stores the current data used to compute the next random value. Based on
// the prng here: http://burtleburtle.net/bob/rand/smallprng.html.
///
struct FSRandCtx {
    u32 a, b, c, d;
};

// Returns a seed which provides sufficient sub-millisecond movement.
u32 fsGetRoughSeed(void);

// Return the next u32 in the PRNG sequence.
u32 fsRandNext(FSRandCtx *ctx);

// Seed the random context.
void fsRandSeed(FSRandCtx *ctx, u32 seed);

// Retrieve the next random piece in the queue for the specified engine.
FSBlock fsNextRandomPiece(FSEngine *f);

#endif // FS_RAND_H
