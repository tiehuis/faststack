///
// fsTables.c
// ==========
//
// A rotation system consists of wallkicks and initial piece offsets.
//
// We index the kick tables using the followwing ordering:
//  * I, J, L, O, S, T, Z
//
// All tables are computed from their beginning direction. To compute the
// wallkick for the rotation `0 -> R` we would check table.kicksR[0];
///

#include "fs.h"
#include "fsInternal.h"

// The C99 standard specifies that static variables are 0 initialized
// (section 6.7.8 item 10) which we rely on during initialization.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// Shorthand for specifying the end of a wallkick table.
#define WKE {.z = WK_END}

// Shorthand for specifying an empty kick table.
#define NO_KICKS -1, -1, -1, -1, -1, -1, -1

const WallkickTable emptyWallkickTable = {
    {{0, 0}, WKE}, // R -> U
    {{0, 0}, WKE}, // D -> R
    {{0, 0}, WKE}, // L -> D
    {{0, 0}, WKE}  // U -> L
};

///
// Simple
//
// Performs no wallkicks.
///
static const FSRotationSystem rotSimple = {
    .kicksR = {NO_KICKS},
    .kicksL = {NO_KICKS},
    .kicksH = {NO_KICKS}
};

///
// SRS
//
// Performs wallkicks adhering to the Super Rotation System.
///
static const FSRotationSystem rotSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // R -> 0
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}, WKE}, // 2 -> R
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}, WKE}  // L -> 2
        }
    }
};

///
// Arika SRS
//
// Similar to SRS with a different set of I wallkicks.
///
static const FSRotationSystem rotArikaSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, { 1,-2}, {-2, 1}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 1}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, { 2, 0}, {-1, 0}, {-1,-2}, { 2, 1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // R -> 0
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2,-1}, { 1, 1}, WKE}, // 2 -> R
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 2}, WKE}  // L -> 2
        }
    }
};

///
// TGM1/2
//
// An extension of the Sega rotation system.
///
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

///
// DTET
//
// A symmetric system from the DTET series of games. Can be considered a
// simplification of the TGM1/2 rotation system.
///
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

///
// List of all available rotation systems.
///
const FSRotationSystem *rotationSystems[FS_NRS] = {
    &rotSimple, &rotSRS, &rotArikaSRS, &rotTGM12, &rotDTET
};
