///
// fsRand.c
// ========
//
// Implements a number of different types of randomizers.
//
// All randomizer can uset the internal 'FSGame' variables
// 'randomInternal' and 'randomInternalIndex'.
//
// The PRNG used is found here: http://burtleburtle.net/bob/rand/smallprng.html.
//
// We do not use stdlib's implementation so that we can ensure that we
// regenerate specific piece sequences across any platform just from an initial
// seed.
///

#include "fs.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

///
// Generate the next value for this PRNG context.
uint32_t fsRandNext(FSRandCtx *ctx)
{
#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

    const uint32_t e = ctx->a - ROT(ctx->b, 27);
    ctx->a = ctx->b ^ ROT(ctx->c, 17);
    ctx->b = ctx->c + ctx->d;
    ctx->c = ctx->d + e;
    ctx->d = e + ctx->a;
    return ctx->d;

#undef ROT
}

///
// Generate an unbiased integer within the range [low, high).
static uint32_t fsRandInRange(FSRandCtx *ctx, uint32_t low, uint32_t high)
{
    const uint32_t range = high - low;
    const uint32_t rem = UINT32_MAX % range;
    uint32_t x;

    do {
        x = fsRandNext(ctx);
    } while (x >= UINT32_MAX - rem);

    return low + x % range;
}

///
// Seed the randomizer.
void fsRandSeed(FSRandCtx *ctx, uint32_t seed)
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
static void initNoSZOBag7(FSGame *f)
{
    f->randomInternalIndex = 0;
    for (int i = 0; i < FS_NPT; ++i) {
        f->randomInternal[i] = i;
    }

    do {
        fisherYatesShuffle(&f->randomContext, f->randomInternal, FS_NPT);
        // Discard S, Z, O pieces
    } while (f->randomInternal[0] == FS_S ||
             f->randomInternal[0] == FS_Z ||
             f->randomInternal[0] == FS_O);
}

static FSBlock fromNoSZOBag7(FSGame *f)
{
    const FSBlock b = f->randomInternal[f->randomInternalIndex];
    if (++f->randomInternalIndex == FS_NPT) {
        f->randomInternalIndex = 0;
        fisherYatesShuffle(&f->randomContext, f->randomInternal, FS_NPT);
    }
    return b;
}

///
// Simple Randomizer.
//
// A simple randomizer just generates a random number with no knowledge
// of what comes before or after it.
///
static FSBlock fromSimple(FSGame *f)
{
    (void) f;
    return fsRandInRange(&f->randomContext, 0, FS_NPT);
}

///
// TGM1 Randomizer.
//
// Simple 4-roll randomizer with initial 4 Z history.
///
static void initTGM1(FSGame *f)
{
    // Fill history with 4 Z's
    f->randomInternal[0] = FS_Z;
    f->randomInternal[1] = FS_Z;
    f->randomInternal[2] = FS_Z;
    f->randomInternal[3] = FS_Z;

    // We use a circular buffer to manage history
    f->randomInternalIndex = 0;
}

static FSBlock fromTGM1or2(FSGame *f, int noOfRolls)
{
    FSBlock piece = 0;
    for (int i = 0; i < noOfRolls; ++i) {
        piece = fsRandInRange(&f->randomContext, 0, FS_NPT);

        // If the piece is not in the queue then we are done
        int j;
        for (j = 0; j < 4; ++j) {
            if (f->randomInternal[j] == piece) {
                break;
            }
        }

        // Failed to find the piece in history, break early
        if (j == 4) {
            break;
        }
    }

    // Update history with current piece
    f->randomInternal[f->randomInternalIndex] = piece;
    f->randomInternalIndex = (f->randomInternalIndex + 1) & 3;
    fsLogDebug("Returning piece: %d", piece);
    return piece;
}

///
// TGM2 Randomizer.
//
// This only differs from the TGM1 in the initial history.
// Reuse the `fromTGM1or2` function to generate pieces.
///
static void initTGM2(FSGame *f)
{
    // Fill history with Z, S, S, Z
    f->randomInternal[0] = FS_Z;
    f->randomInternal[1] = FS_S;
    f->randomInternal[2] = FS_S;
    f->randomInternal[3] = FS_Z;

    // We use a circular buffer to manage history
    f->randomInternalIndex = 0;
}

///
// Generate the next random piece in sequence using the games randomizer.
//
// This wil initialize the randomizer if it has yet to be called with the
// current randomizer type.
///
FSBlock fsNextRandomPiece(FSGame *f)
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
