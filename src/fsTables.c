///
// fsTables.c
//
// Define all rotation systems.
//
// A rotation system consists of wallkicks and initial piece offsets.
// Ensure the ordering of the tables matches the values as defined by
// BlockType! As we use the numbering to implicitly index.
//
// Rotations can utilize the field state by specifying a new flag in the
// z column and then implementing the required case in fsDoRotate. See
// the WK_ARIKA_LJ rule for an example.
//
// Note the index ordering is based on the following in fs.h:
//
//  - I, J, L, O, S, T, Z
//
// All tables are computed from the start direction. i.e. if we want the wallkick
// from 0 -> R, then we check the table[theta] and not table[newDir].
// This seems easier to think about in terms of symmetry (choosing right or left)
// indexes into the same position, just a different table.

#include "fs.h"
#include "fsInternal.h"

// The C standard dictates that static members are zero-filled by default.
// We implicitly set the .z extra value to 0, but we get a warning under
// strict checking, so disable this here.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// Shorthand for specifying final wallkick test.
#define WKE {.z = WK_END}

// Shorthand for a kick table index with no kicks.
#define NO_KICKS -1, -1, -1, -1, -1, -1, -1

// This cannot be static since it is externed.
const WallkickTable emptyWallkickTable = {
    {{0, 0}, WKE}, // R -> U
    {{0, 0}, WKE}, // D -> R
    {{0, 0}, WKE}, // L -> D
    {{0, 0}, WKE}  // U -> L
};

// The simple rotation system does nothing but is useful when encountering
// an empty wallkick test.
static const FSRotationSystem rotSimple = {
    .kicksR = {NO_KICKS},
    .kicksL = {NO_KICKS},
    .kicksH = {NO_KICKS}
};

// Standard SRS
static const FSRotationSystem rotSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2,-1}, { 1, 2}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1, 2}, { 2,-1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2, 1}, {-1,-2}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1, 2}, { 2,-1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2, 1}, {-1,-2}, WKE}, // R -> 0
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 1}, WKE}, // 2 -> R
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2,-1}, { 1, 2}, WKE}  // L -> 2
        }
    }
};

// Arika SRS mirrors standard SRS with a different I block rotation pattern.
static const FSRotationSystem rotArikaSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, { 1, 2}, {-2,-1}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1, 2}, { 2,-1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2, 1}, {-1,-1}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, { 2, 0}, {-1, 0}, {-1, 2}, { 2,-1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2, 1}, {-1,-2}, WKE}, // R -> 0
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-1}, WKE}, // 2 -> R
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-2}, WKE}  // L -> 2
        }
    }
};

// Rotation system of TGM1 and TGM2.
// The L, J and T tetriminos have a special case depending on the arrangement
// of the field so we handle this on certain rotations.
static const FSRotationSystem rotTGM12 = {
    .kicksR = {0, 0, 0, -1, 0, 0, 0},
    .kicksL = {0, 0, 0, -1, 0, 0, 0},
    .kicksH = {NO_KICKS},
    .kickTables = {
        {
            {{ 0, 0}, { 1, 0}, {.z = WK_ARIKA_LJT}, {-1, 0}, WKE}, // 0
            {{ 0, 0}, { 1, 0},                      {-1, 0}, WKE}, // R
            {{ 0, 0}, { 1, 0}, {.z = WK_ARIKA_LJT}, {-1, 0}, WKE}, // 2
            {{ 0, 0}, { 1, 0},                      {-1, 0}, WKE}, // L
        },
    }
};

// DTET rotation is a simple symmetric rotation scheme with reversed entry orders.
// It could be considered an extension of TGM with some exceptions removed.
static const FSRotationSystem rotDTET = {
    .entryTheta = {0, 2, 2, 0, 0, 2, 0},
    .kicksR = {0, 0, 0, -1, 0, 0, 0},
    .kicksL = {1, 1, 1, -1, 1, 1, 1},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: Clockwise
        {
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // O -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}  // L -> 0
        },

        // 1: Anticlockwise
        {
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // 0 -> L
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}  // L -> 2
        }
    }
};

/// Set of known rotation systems.
//
// This is externed in `fs.h`
const FSRotationSystem *rotationSystems[FS_NRS] = {
    &rotSimple, &rotSRS, &rotArikaSRS, &rotTGM12, &rotDTET
};
