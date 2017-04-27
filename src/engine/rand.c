///
// rand.c
// ======
//
// Implements a number of different types of randomizers.
//
// All randomizer can uset the internal 'FSEngine' variables
// 'randBuf' and 'randBufIndex'.
//
// The PRNG used is found here: http://burtleburtle.net/bob/rand/smallprng.html.
//
// We do not use stdlib's implementation so that we can ensure that we
// regenerate specific piece sequences across any platform just from an initial
// seed.
///

#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "rand.h"
#include "log.h"

///
// Return a decent seed value.
//
// This is only intended to change often enough to be recalculated on
// game restart.
u32 fsGetRoughSeed(void)
{
    return time(NULL) * clock();
}

///
// Generate the next value for this PRNG context.
u32 fsRandNext(FSRandCtx *ctx)
{
#define R(x, k) (((x) << (k)) | ((x) >> (32 - (k))))
    const u32 e = ctx->a - R(ctx->b, 27);
    ctx->a = ctx->b ^ R(ctx->c, 17);
    ctx->b = ctx->c + ctx->d;
    ctx->c = ctx->d + e;
    ctx->d = e + ctx->a;
    return ctx->d;
#undef R
}

///
// Generate an unbiased integer within the range [low, high).
static u32 fsRandInRange(FSRandCtx *ctx, u32 low, u32 high)
{
    const u32 range = high - low;
    const u32 rem = UINT32_MAX % range;
    u32 x;

    do {
        x = fsRandNext(ctx);
    } while (x >= UINT32_MAX - rem);

    return low + x % range;
}

///
// Seed the randomizer.
void fsRandSeed(FSRandCtx *ctx, u32 seed)
{
    ctx->a = 0xf1ea5eed;
    ctx->b = ctx->c = ctx->d = seed;
    for (int i = 0; i < 20; ++i) {
        (void) fsRandNext(ctx);
    }
}

///
// Perform an unbiased shuffle.
///
static void fisherYatesShuffle(FSRandCtx *ctx, FSBlock *a, int n)
{
    for (int i = n - 1; i > 0; --i) {
        const int j = fsRandInRange(ctx, 0, i + 1);
        const int t = a[j];
        a[j] = a[i];
        a[i] = t;
    }
}

///
// Bag Randomizer (no SZO)
//
// This implements a standard 7-bag shuffle randomizer. An extra condition is
// added to ensure that an S, Z or O piece is not dealt first.
///
static void initNoSZOBag7(FSEngine *f)
{
    f->randBufIndex = 0;
    for (int i = 0; i < FS_NPT; ++i) {
        f->randBuf[i] = i;
    }

    do {
        fisherYatesShuffle(&f->randomContext, f->randBuf, FS_NPT);
        // Discard S, Z, O pieces
    } while (f->randBuf[0] == FS_S ||
             f->randBuf[0] == FS_Z ||
             f->randBuf[0] == FS_O);
}

static FSBlock fromNoSZOBag7(FSEngine *f)
{
    const FSBlock b = f->randBuf[f->randBufIndex];
    if (++f->randBufIndex == FS_NPT) {
        f->randBufIndex = 0;
        fisherYatesShuffle(&f->randomContext, f->randBuf, FS_NPT);
    }
    return b;
}

///
// Simple Randomizer.
//
// A simple randomizer just generates a random number with no knowledge
// of what comes before or after it.
///
static FSBlock fromSimple(FSEngine *f)
{
    (void) f;
    return fsRandInRange(&f->randomContext, 0, FS_NPT);
}

///
// TGM1 Randomizer.
//
// Simple 4-roll randomizer with initial 4 Z history.
///
static void initTGM1(FSEngine *f)
{
    f->randBuf[0] = FS_Z;
    f->randBuf[1] = FS_Z;
    f->randBuf[2] = FS_Z;
    f->randBuf[3] = FS_Z;
    f->randBufIndex = 0;
}

static FSBlock fromTGM1or2(FSEngine *f, int noOfRolls)
{
    FSBlock piece = 0;
    for (int i = 0; i < noOfRolls; ++i) {
        piece = fsRandInRange(&f->randomContext, 0, FS_NPT);

        // If the piece is not in the queue then we are done
        int j;
        for (j = 0; j < 4; ++j) {
            if (f->randBuf[j] == piece) {
                break;
            }
        }

        // Failed to find the piece in history, break early
        if (j == 4) {
            break;
        }
    }

    f->randBuf[f->randBufIndex] = piece;
    f->randBufIndex = (f->randBufIndex + 1) & 3;
    fsLogDebug("Returning piece: %d", piece);
    return piece;
}

///
// TGM2 Randomizer.
//
// This only differs from the TGM1 in the initial history.
// Reuse the `fromTGM1or2` function to generate pieces.
///
static void initTGM2(FSEngine *f)
{
    f->randBuf[0] = FS_Z;
    f->randBuf[1] = FS_S;
    f->randBuf[2] = FS_S;
    f->randBuf[3] = FS_Z;
    f->randBufIndex = 0;
}

///
// Generate the next random piece in sequence using the games randomizer.
//
// This wil initialize the randomizer if it has yet to be called with the
// current randomizer type.
///
FSBlock fsNextRandomPiece(FSEngine *f)
{
    if (f->randomizer != f->lastRandomizer) {
        f->lastRandomizer = f->randomizer;
        switch (f->randomizer) {
          case FST_RAND_SIMPLE:
            break;
          case FST_RAND_NOSZO_BAG7:
            initNoSZOBag7(f);
            break;
          case FST_RAND_TGM1:
            initTGM1(f);
            break;
          case FST_RAND_TGM2:
            initTGM2(f);
            break;
        }
    }

    switch (f->randomizer) {
      case FST_RAND_SIMPLE:
        return fromSimple(f);
      case FST_RAND_NOSZO_BAG7:
        return fromNoSZOBag7(f);
      case FST_RAND_TGM1:
        return fromTGM1or2(f, 4);
      case FST_RAND_TGM2:
        return fromTGM1or2(f, 6);
      default:
        fsLogFatal("Unknown randomizer: %d", f->randomizer);
        exit(1);
    }
}
