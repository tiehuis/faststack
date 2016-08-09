///
// fsRand.c
//
// Implements a number of different types of randomizers.
//
// All randomizer can uset the internal 'FSGame' variables
// 'randomInternal' and 'randomInternalIndex'.
///

#include <stdlib.h>
#include <stdio.h>
#include "fs.h"

///
// Generate an unbiased random number between [low, high).
//
// Standard technique to unbias a value.
///
static int randomInRange(int low, int high)
{
    const int range = high - low + 1;
    const int rem = RAND_MAX % range;
    int x;

    do {
        x = rand();
    } while (x >= RAND_MAX - rem);

    return low + x % range;
}

///
// Perform an unbiased shuffle.
///
static void fisherYatesShuffle(FSBlock *a, int n)
{
    for (int i = n - 1; i > 0; --i) {
        const int j = randomInRange(0, i + 1);
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
        fisherYatesShuffle(f->randomInternal, FS_NPT);
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
        fisherYatesShuffle(f->randomInternal, FS_NPT);
    }
    return b;
}

///
// Simple Randomizer.
//
// A simple randomizer just generates a random number with no knowledge
// of what comes before or after it.
///
FSBlock fromSimple(FSGame *f)
{
    (void) f;
    return randomInRange(0, FS_NPT);
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
          case FSRAND_SIMPLE:
            break;
          case FSRAND_NOSZO_BAG7:
            initNoSZOBag7(f);
            break;
        }
    }

    switch (f->randomizer) {
      case FSRAND_SIMPLE:
        return fromSimple(f);
      case FSRAND_NOSZO_BAG7:
        return fromNoSZOBag7(f);
      default:
        fsLogFatal("Unknown randomizer: %d", f->randomizer);
        exit(1);
    }

    fsLogFatal("Unreachable code encountered");
}
