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

static void fisherYatesShuffle(FSBlock *a, int n)
{
    for (int i = n - 1; i > 0; --i) {
        const int j = rand() % (i + 1);
        const int t = a[j];
        a[j] = a[i];
        a[i] = t;
    }
}

static void initNoSZOBag7(FSGame *f)
{
    f->randomInternalIndex = 0;
    for (int i = 0; i < 7; ++i) {
        f->randomInternal[i] = i;
    }

    do {
        fisherYatesShuffle(f->randomInternal, 7);
        // Discard S, Z, O pieces
    } while (f->randomInternal[0] == FS_S ||
             f->randomInternal[0] == FS_Z ||
             f->randomInternal[0] == FS_O);
}

FSBlock fromNoSZOBag7(FSGame *f)
{
    const FSBlock b = f->randomInternal[f->randomInternalIndex];
    if (++f->randomInternalIndex == 7) {
        f->randomInternalIndex = 0;
        fisherYatesShuffle(f->randomInternal, 7);
    }
    return b;
}

FSBlock fromSimple(FSGame *f)
{
    (void) f;
    return rand() % 7;
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

    // Unreachable!
}
