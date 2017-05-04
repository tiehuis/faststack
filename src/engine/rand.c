///
// rand.c
// ======
//
// Implements a number of different types of randomizers.
//
// All randomizer can use the internal 'FSEngine' variables
// 'randBuf, 'randBufIndex' and 'randBufExtra'.
//
// The PRNG used is found here: http://burtleburtle.net/bob/rand/smallprng.html.
//
// We do not use stdlib's implementation so that we can ensure that we
// regenerate specific piece sequences across any platform just from an initial
// seed.
///

#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
//
// This can be used for sub-single bag randomizers.
///
static void initBag(FSEngine *f)
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

// `length` **MUST** be less than `FS_NPT`.
static FSBlock fromBag(FSEngine *f, int length, bool checkSeam)
{
    const FSBlock b = f->randBuf[f->randBufIndex];
    if (++f->randBufIndex == length) {
        f->randBufIndex = 0;
        fisherYatesShuffle(&f->randomContext, f->randBuf, FS_NPT);

        if (checkSeam) {
            // If there was a duplicate across seams, swap the head with a
            // random another random piece in the bag.
            if (b == f->randBuf[f->randBufIndex]) {
                const int index = fsRandInRange(&f->randomContext, 1, FS_NPT);
                const FSBlock tmp = f->randBuf[index];
                f->randBuf[index] = f->randBuf[0];
                f->randBuf[0] = tmp;
            }
        }
    }

    return b;
}

///
// Multi Bag Randomizer
//
// Implements a set of bag randomizer combined then shuffled. This increases
// the variance between pieces while still retaining some semblance of
// determinism.
static void initMultiBag(FSEngine *f, int bagCount)
{
    f->randBufIndex = 0;
    for (int i = 0; i < bagCount * FS_NPT; ++i) {
        f->randBuf[i] = i % FS_NPT;
    }

    do {
        fisherYatesShuffle(&f->randomContext, f->randBuf, bagCount * FS_NPT);
        // Discard S, Z, O pieces
    } while (f->randBuf[0] == FS_S ||
             f->randBuf[0] == FS_Z ||
             f->randBuf[0] == FS_O);
}

static FSBlock fromMultiBag(FSEngine *f, int bagCount)
{
    const FSBlock b = f->randBuf[f->randBufIndex];
    if (++f->randBufIndex == bagCount * FS_NPT) {
        f->randBufIndex = 0;
        fisherYatesShuffle(&f->randomContext, f->randBuf, bagCount * FS_NPT);
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
//
// The extra field is used to handle the first roll special case.
///
static void initTGM1(FSEngine *f)
{
    f->randBuf[0] = FS_Z;
    f->randBuf[1] = FS_Z;
    f->randBuf[2] = FS_Z;
    f->randBuf[3] = FS_Z;
    f->randBufIndex = 0;
    f->randBufExtra[0] = 0;
}

// TODO: Wrong variance calculated for TGM2 6 roll variant.
static FSBlock fromTGM1or2(FSEngine *f, int noOfRolls)
{
    assert(noOfRolls > 0);

    if (!f->randBufExtra[0]) {
        f->randBufExtra[0] = 1;
        const FSBlock choice[] = { FS_J, FS_I, FS_L, FS_T };
        return choice[fsRandInRange(&f->randomContext, 0, sizeof(choice))];
    }

    FSBlock piece;
    for (int i = 0; i < noOfRolls; ++i) {
        // TODO: 'vectorize' this by generating four pieces at once to speed
        // up by a factor of four/six. Slow for testing.
        piece = fsRandInRange(&f->randomContext, 0, FS_NPT);

        // If the piece is not in the history then we are done
        if (piece != f->randBuf[0]
             && piece != f->randBuf[1]
             && piece != f->randBuf[2]
             && piece != f->randBuf[3]) {
            break;
        }
    }

    f->randBuf[f->randBufIndex] = piece;
    f->randBufIndex = (f->randBufIndex + 1) & 3;
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
    f->randBufExtra[0] = 0;
}

///
// TGM3 Randomizer.
//
// This uses a bag of 35 which fills incrementally.
//
// The extra random buffer is used in the following way:
//
//  [0..3]  = History
//  [4..10] = Current Drought Order
//  [11]    = Flag indicating which pieces have been seen for bug emulation
//  [12]    = Whether this is the first roll
//
// The history index uses `randBufIndex`.
static void initTGM3(FSEngine *f)
{
    for (int i = 0; i < 35; ++i) {
        f->randBuf[i] = i % FS_NPT;
    }

    // Pre-fill history
    f->randBufExtra[0] = FS_S;
    f->randBufExtra[1] = FS_Z;
    f->randBufExtra[2] = FS_S;
    f->randBufExtra[3] = FS_Z;
    f->randBufIndex = 0;

    // Pre-fill drought order
    f->randBufExtra[4]  = FS_J;
    f->randBufExtra[5]  = FS_I;
    f->randBufExtra[6]  = FS_Z;
    f->randBufExtra[7]  = FS_L;
    f->randBufExtra[8]  = FS_O;
    f->randBufExtra[9]  = FS_T;
    f->randBufExtra[10] = FS_S;

    // Seen count
    f->randBufExtra[11] = 0;

    // Is this the first roll?
    f->randBufExtra[12] = 0;
}

// This is a 6-roll system with bias towards pieces which have not recently
// dropped.
static FSBlock fromTGM3(FSEngine *f)
{
    FSBlock piece;
    int index;

    // First roll is a special case.
    if (!f->randBufExtra[12]) {
        f->randBufExtra[12] = 1;
        const FSBlock choice[] = { FS_J, FS_I, FS_L, FS_T };
        piece = choice[fsRandInRange(&f->randomContext, 0, 4)];
    }
    else {
        int roll;
        for (roll = 0; roll < 6; ++roll) {
            index = fsRandInRange(&f->randomContext, 0, 35);
            piece = f->randBuf[index];

            // If the piece is not in the history then we are done
            if (piece != (FSBlock) f->randBufExtra[0]
                 && piece != (FSBlock) f->randBufExtra[1]
                 && piece != (FSBlock) f->randBufExtra[2]
                 && piece != (FSBlock) f->randBufExtra[3]) {
                break;
            }

            // Update the bag to bias against the current least-common piece.
            if (roll < 5) {
                f->randBuf[index] = f->randBufExtra[4];
            }
        }

        // Mark the piece as seen
        f->randBufExtra[11] |= (1 << piece);

        // The bag is not updated in the case that every piece has been seen, a
        // reroll occurred on the piece and we just chose the most droughted
        // piece.
        const bool bug = roll > 0 &&
                   piece == (FSBlock) f->randBufExtra[4] &&
                   f->randBufExtra[11] == ((1 << 7) - 1);

        if (!bug) {
            f->randBuf[index] = f->randBufExtra[4];
        }

        // Put current drought piece to back of the drought queue.
        for (int i = 0; i < FS_NPT; ++i) {
            if (piece == (FSBlock) f->randBufExtra[4 + i]) {
                memcpy(&f->randBufExtra[4 + i], &f->randBufExtra[5 + i], 6 - i);
                f->randBufExtra[10] = piece;
                break;
            }
        }
    }

    // Update the history with the new piece.
    f->randBufExtra[f->randBufIndex] = piece;
    f->randBufIndex = (f->randBufIndex + 1) & 3;
    return piece;
}

static void initRandomizer(FSEngine *f)
{
    switch (f->randomizer) {
        case FST_RAND_SIMPLE:
            break;
        case FST_RAND_BAG7:
        case FST_RAND_BAG7_SEAM_CHECK:
        case FST_RAND_BAG6:
            initBag(f);
            break;
        case FST_RAND_MULTI_BAG2:
            initMultiBag(f, 2);
            break;
        case FST_RAND_MULTI_BAG4:
            initMultiBag(f, 4);
            break;
        case FST_RAND_MULTI_BAG9:
            initMultiBag(f, 9);
            break;
        case FST_RAND_TGM1:
            initTGM1(f);
            break;
        case FST_RAND_TGM2:
            initTGM2(f);
            break;
        case FST_RAND_TGM3:
            initTGM3(f);
            break;
    }
}

///
// Generate the next random piece in sequence using the games randomizer.
//
// This wil initialize the randomizer if it has yet to be called with the
// current randomizer type.
//
// Theoretically we could switch randomizers mid-game with no trouble however
// this would require extra tweaks for replay management.
///
FSBlock fsNextRandomPiece(FSEngine *f)
{
    if (f->randomizer != f->lastRandomizer) {
        f->lastRandomizer = f->randomizer;
        initRandomizer(f);
    }

    switch (f->randomizer) {
        case FST_RAND_SIMPLE:
            return fromSimple(f);
        case FST_RAND_BAG7:
            return fromBag(f, 7, false);
        case FST_RAND_TGM1:
            return fromTGM1or2(f, 4);
        case FST_RAND_TGM2:
            return fromTGM1or2(f, 6);
        case FST_RAND_TGM3:
            return fromTGM3(f);
        case FST_RAND_BAG7_SEAM_CHECK:
            return fromBag(f, 7, true);
        case FST_RAND_BAG6:
            return fromBag(f, 6, false);
        case FST_RAND_MULTI_BAG2:
            return fromMultiBag(f, 2);
        case FST_RAND_MULTI_BAG4:
            return fromMultiBag(f, 4);
        case FST_RAND_MULTI_BAG9:
            return fromMultiBag(f, 9);
        default:
            fsLogFatal("Unknown randomizer: %d", f->randomizer);
            exit(1);
    }
}
